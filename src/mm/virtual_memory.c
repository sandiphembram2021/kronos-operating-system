#include "kronos.h"

/* Advanced Virtual Memory Management for Kronos OS */

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFFFFFFFFFFFF000
#define PAGE_OFFSET_MASK 0x0FFF
#define PAGES_PER_TABLE 512
#define KERNEL_VIRTUAL_BASE 0xFFFFFFFF80000000
#define USER_VIRTUAL_BASE 0x400000
#define USER_STACK_TOP 0x7FFFFFFFFFFF
#define HEAP_START 0x600000

/* Page flags */
#define PAGE_PRESENT    0x001
#define PAGE_WRITABLE   0x002
#define PAGE_USER       0x004
#define PAGE_WRITETHROUGH 0x008
#define PAGE_CACHE_DISABLE 0x010
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_SIZE_FLAG  0x080
#define PAGE_GLOBAL     0x100
#define PAGE_COW        0x200  /* Copy-on-write */
#define PAGE_SWAPPED    0x400  /* Page is swapped out */

/* Virtual Memory Area (VMA) */
struct vma {
    u64 start;
    u64 end;
    u32 flags;
    u32 permissions;
    struct file* file;  /* For memory-mapped files */
    u64 file_offset;
    struct vma* next;
};

/* Memory mapping permissions */
#define PROT_READ   0x1
#define PROT_WRITE  0x2
#define PROT_EXEC   0x4
#define PROT_NONE   0x0

/* Memory mapping flags */
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_FIXED     0x10
#define MAP_ANONYMOUS 0x20

/* Page table entry */
typedef u64 pte_t;
typedef u64 pmd_t;
typedef u64 pud_t;
typedef u64 pgd_t;

/* Page directory structure */
struct page_directory {
    pgd_t* pgd;
    u64 physical_addr;
    u32 ref_count;
};

/* Memory statistics */
struct memory_stats {
    u64 total_pages;
    u64 free_pages;
    u64 used_pages;
    u64 cached_pages;
    u64 swap_pages;
    u64 kernel_pages;
    u64 user_pages;
};

/* Physical page frame */
struct page_frame {
    u64 physical_addr;
    u32 ref_count;
    u32 flags;
    struct page_frame* next;
};

/* Memory management state */
static struct {
    struct page_frame* free_pages;
    struct page_frame* page_frames;
    u64 total_memory;
    u64 available_memory;
    struct memory_stats stats;
    bool paging_enabled;
} mm_state;

/* Swap management */
#define MAX_SWAP_PAGES 65536
static struct {
    bool* swap_bitmap;
    u32 total_swap_pages;
    u32 used_swap_pages;
    struct file* swap_file;
} swap_state;

/* Initialize virtual memory management */
void vmm_init(void) {
    /* Initialize physical memory manager */
    pmm_init();
    
    /* Set up kernel page tables */
    setup_kernel_paging();
    
    /* Initialize swap system */
    swap_init();
    
    mm_state.paging_enabled = true;
    
    vga_puts("Virtual memory management initialized\n");
}

/* Physical Memory Manager */
void pmm_init(void) {
    /* Get memory map from bootloader */
    u64 memory_size = get_memory_size();
    u64 num_pages = memory_size / PAGE_SIZE;
    
    /* Allocate page frame array */
    mm_state.page_frames = (struct page_frame*)kmalloc(num_pages * sizeof(struct page_frame));
    
    /* Initialize free page list */
    mm_state.free_pages = NULL;
    
    for (u64 i = 0; i < num_pages; i++) {
        struct page_frame* frame = &mm_state.page_frames[i];
        frame->physical_addr = i * PAGE_SIZE;
        frame->ref_count = 0;
        frame->flags = 0;
        
        /* Add to free list if not reserved */
        if (!is_memory_reserved(frame->physical_addr)) {
            frame->next = mm_state.free_pages;
            mm_state.free_pages = frame;
            mm_state.stats.free_pages++;
        }
    }
    
    mm_state.total_memory = memory_size;
    mm_state.stats.total_pages = num_pages;
}

/* Allocate physical page */
u64 pmm_alloc_page(void) {
    if (!mm_state.free_pages) {
        return 0;  /* Out of memory */
    }
    
    struct page_frame* frame = mm_state.free_pages;
    mm_state.free_pages = frame->next;
    
    frame->ref_count = 1;
    frame->next = NULL;
    
    mm_state.stats.free_pages--;
    mm_state.stats.used_pages++;
    
    /* Clear page */
    memset((void*)frame->physical_addr, 0, PAGE_SIZE);
    
    return frame->physical_addr;
}

/* Free physical page */
void pmm_free_page(u64 physical_addr) {
    u64 page_index = physical_addr / PAGE_SIZE;
    struct page_frame* frame = &mm_state.page_frames[page_index];
    
    if (frame->ref_count > 0) {
        frame->ref_count--;
        
        if (frame->ref_count == 0) {
            /* Add back to free list */
            frame->next = mm_state.free_pages;
            mm_state.free_pages = frame;
            
            mm_state.stats.free_pages++;
            mm_state.stats.used_pages--;
        }
    }
}

/* Page Table Management */

/* Get page table entry */
pte_t* get_pte(pgd_t* pgd, u64 virtual_addr, bool create) {
    u64 pgd_index = (virtual_addr >> 39) & 0x1FF;
    u64 pud_index = (virtual_addr >> 30) & 0x1FF;
    u64 pmd_index = (virtual_addr >> 21) & 0x1FF;
    u64 pte_index = (virtual_addr >> 12) & 0x1FF;
    
    /* Check PGD entry */
    if (!(pgd[pgd_index] & PAGE_PRESENT)) {
        if (!create) return NULL;
        
        u64 pud_phys = pmm_alloc_page();
        if (!pud_phys) return NULL;
        
        pgd[pgd_index] = pud_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    }
    
    pud_t* pud = (pud_t*)(pgd[pgd_index] & PAGE_MASK);
    
    /* Check PUD entry */
    if (!(pud[pud_index] & PAGE_PRESENT)) {
        if (!create) return NULL;
        
        u64 pmd_phys = pmm_alloc_page();
        if (!pmd_phys) return NULL;
        
        pud[pud_index] = pmd_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    }
    
    pmd_t* pmd = (pmd_t*)(pud[pud_index] & PAGE_MASK);
    
    /* Check PMD entry */
    if (!(pmd[pmd_index] & PAGE_PRESENT)) {
        if (!create) return NULL;
        
        u64 pte_phys = pmm_alloc_page();
        if (!pte_phys) return NULL;
        
        pmd[pmd_index] = pte_phys | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    }
    
    pte_t* pte_table = (pte_t*)(pmd[pmd_index] & PAGE_MASK);
    
    return &pte_table[pte_index];
}

/* Map virtual page to physical page */
i32 map_page(pgd_t* pgd, u64 virtual_addr, u64 physical_addr, u32 flags) {
    pte_t* pte = get_pte(pgd, virtual_addr, true);
    if (!pte) {
        return -1;  /* Failed to allocate page tables */
    }
    
    *pte = physical_addr | flags;
    
    /* Invalidate TLB entry */
    __asm__ volatile ("invlpg (%0)" :: "r" (virtual_addr) : "memory");
    
    return 0;
}

/* Unmap virtual page */
void unmap_page(pgd_t* pgd, u64 virtual_addr) {
    pte_t* pte = get_pte(pgd, virtual_addr, false);
    if (pte && (*pte & PAGE_PRESENT)) {
        u64 physical_addr = *pte & PAGE_MASK;
        *pte = 0;
        
        /* Free physical page */
        pmm_free_page(physical_addr);
        
        /* Invalidate TLB entry */
        __asm__ volatile ("invlpg (%0)" :: "r" (virtual_addr) : "memory");
    }
}

/* Virtual Memory Areas (VMAs) */

/* Create new VMA */
struct vma* vma_create(u64 start, u64 end, u32 permissions, u32 flags) {
    struct vma* vma = (struct vma*)kmalloc(sizeof(struct vma));
    if (!vma) {
        return NULL;
    }
    
    vma->start = start;
    vma->end = end;
    vma->permissions = permissions;
    vma->flags = flags;
    vma->file = NULL;
    vma->file_offset = 0;
    vma->next = NULL;
    
    return vma;
}

/* Find VMA containing address */
struct vma* vma_find(struct process* proc, u64 addr) {
    struct vma* vma = proc->vma_list;
    
    while (vma) {
        if (addr >= vma->start && addr < vma->end) {
            return vma;
        }
        vma = vma->next;
    }
    
    return NULL;
}

/* Memory Mapping */

/* Map memory region */
void* mmap(void* addr, size_t length, i32 prot, i32 flags, i32 fd, off_t offset) {
    struct process* current = get_current_process();
    if (!current) {
        return MAP_FAILED;
    }
    
    /* Align to page boundaries */
    u64 start_addr = (u64)addr;
    u64 aligned_length = (length + PAGE_SIZE - 1) & PAGE_MASK;
    
    /* Find suitable virtual address if not fixed */
    if (!(flags & MAP_FIXED)) {
        start_addr = find_free_vma_space(current, aligned_length);
        if (!start_addr) {
            return MAP_FAILED;
        }
    }
    
    /* Create VMA */
    struct vma* vma = vma_create(start_addr, start_addr + aligned_length, prot, flags);
    if (!vma) {
        return MAP_FAILED;
    }
    
    /* Handle file mapping */
    if (!(flags & MAP_ANONYMOUS) && fd >= 0) {
        vma->file = get_file_by_fd(fd);
        vma->file_offset = offset;
    }
    
    /* Add to process VMA list */
    vma->next = current->vma_list;
    current->vma_list = vma;
    
    /* Map pages */
    for (u64 vaddr = start_addr; vaddr < start_addr + aligned_length; vaddr += PAGE_SIZE) {
        u64 paddr = pmm_alloc_page();
        if (!paddr) {
            /* Cleanup on failure */
            munmap((void*)start_addr, aligned_length);
            return MAP_FAILED;
        }
        
        u32 page_flags = PAGE_PRESENT | PAGE_USER;
        if (prot & PROT_WRITE) page_flags |= PAGE_WRITABLE;
        
        map_page(current->page_directory->pgd, vaddr, paddr, page_flags);
    }
    
    return (void*)start_addr;
}

/* Unmap memory region */
i32 munmap(void* addr, size_t length) {
    struct process* current = get_current_process();
    if (!current) {
        return -1;
    }
    
    u64 start_addr = (u64)addr & PAGE_MASK;
    u64 end_addr = ((u64)addr + length + PAGE_SIZE - 1) & PAGE_MASK;
    
    /* Find and remove VMAs */
    struct vma* prev = NULL;
    struct vma* vma = current->vma_list;
    
    while (vma) {
        if (vma->start >= start_addr && vma->end <= end_addr) {
            /* Unmap pages */
            for (u64 vaddr = vma->start; vaddr < vma->end; vaddr += PAGE_SIZE) {
                unmap_page(current->page_directory->pgd, vaddr);
            }
            
            /* Remove from list */
            if (prev) {
                prev->next = vma->next;
            } else {
                current->vma_list = vma->next;
            }
            
            struct vma* next = vma->next;
            kfree(vma);
            vma = next;
        } else {
            prev = vma;
            vma = vma->next;
        }
    }
    
    return 0;
}

/* Page Fault Handler */
void page_fault_handler(u64 fault_addr, u32 error_code) {
    struct process* current = get_current_process();
    if (!current) {
        return;
    }
    
    /* Find VMA containing fault address */
    struct vma* vma = vma_find(current, fault_addr);
    if (!vma) {
        /* Segmentation fault */
        signal_send(current->pid, SIGSEGV);
        return;
    }
    
    /* Check permissions */
    if ((error_code & 0x2) && !(vma->permissions & PROT_WRITE)) {
        /* Write to read-only page */
        signal_send(current->pid, SIGSEGV);
        return;
    }
    
    /* Handle copy-on-write */
    pte_t* pte = get_pte(current->page_directory->pgd, fault_addr, false);
    if (pte && (*pte & PAGE_COW)) {
        handle_cow_fault(fault_addr, pte);
        return;
    }
    
    /* Handle swapped page */
    if (pte && (*pte & PAGE_SWAPPED)) {
        handle_swap_fault(fault_addr, pte);
        return;
    }
    
    /* Allocate new page */
    u64 page_addr = fault_addr & PAGE_MASK;
    u64 physical_page = pmm_alloc_page();
    if (!physical_page) {
        /* Out of memory */
        signal_send(current->pid, SIGKILL);
        return;
    }
    
    u32 flags = PAGE_PRESENT | PAGE_USER;
    if (vma->permissions & PROT_WRITE) flags |= PAGE_WRITABLE;
    
    map_page(current->page_directory->pgd, page_addr, physical_page, flags);
}

/* Copy-on-Write handling */
void handle_cow_fault(u64 fault_addr, pte_t* pte) {
    u64 old_physical = *pte & PAGE_MASK;
    u64 new_physical = pmm_alloc_page();
    
    if (!new_physical) {
        /* Out of memory */
        signal_send(get_current_process()->pid, SIGKILL);
        return;
    }
    
    /* Copy page content */
    memcpy((void*)new_physical, (void*)old_physical, PAGE_SIZE);
    
    /* Update page table entry */
    *pte = new_physical | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    
    /* Decrease reference count of old page */
    pmm_free_page(old_physical);
    
    /* Invalidate TLB */
    __asm__ volatile ("invlpg (%0)" :: "r" (fault_addr) : "memory");
}

/* Swap Management */
void swap_init(void) {
    swap_state.total_swap_pages = MAX_SWAP_PAGES;
    swap_state.used_swap_pages = 0;
    swap_state.swap_bitmap = (bool*)kmalloc(MAX_SWAP_PAGES * sizeof(bool));
    
    /* Initialize swap bitmap */
    for (u32 i = 0; i < MAX_SWAP_PAGES; i++) {
        swap_state.swap_bitmap[i] = false;
    }
    
    /* Create swap file */
    swap_state.swap_file = create_swap_file("/swap", MAX_SWAP_PAGES * PAGE_SIZE);
}

/* Swap out page */
u32 swap_out_page(u64 physical_addr) {
    /* Find free swap slot */
    u32 swap_slot = 0;
    for (u32 i = 0; i < swap_state.total_swap_pages; i++) {
        if (!swap_state.swap_bitmap[i]) {
            swap_slot = i;
            break;
        }
    }
    
    if (swap_slot == 0) {
        return 0;  /* No swap space */
    }
    
    /* Write page to swap */
    u64 swap_offset = swap_slot * PAGE_SIZE;
    if (file_write(swap_state.swap_file, swap_offset, (void*)physical_addr, PAGE_SIZE) != PAGE_SIZE) {
        return 0;
    }
    
    swap_state.swap_bitmap[swap_slot] = true;
    swap_state.used_swap_pages++;
    
    return swap_slot;
}

/* Swap in page */
void handle_swap_fault(u64 fault_addr, pte_t* pte) {
    u32 swap_slot = (*pte >> 12) & 0xFFFFF;  /* Extract swap slot from PTE */
    
    /* Allocate new physical page */
    u64 physical_page = pmm_alloc_page();
    if (!physical_page) {
        signal_send(get_current_process()->pid, SIGKILL);
        return;
    }
    
    /* Read page from swap */
    u64 swap_offset = swap_slot * PAGE_SIZE;
    file_read(swap_state.swap_file, swap_offset, (void*)physical_page, PAGE_SIZE);
    
    /* Update page table entry */
    *pte = physical_page | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    
    /* Free swap slot */
    swap_state.swap_bitmap[swap_slot] = false;
    swap_state.used_swap_pages--;
    
    /* Invalidate TLB */
    __asm__ volatile ("invlpg (%0)" :: "r" (fault_addr) : "memory");
}
