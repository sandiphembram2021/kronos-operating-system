#include "kronos.h"

/* Basic Memory Management */

#define HEAP_START 0x200000  /* 2MB */
#define HEAP_SIZE  0x800000  /* 8MB heap */
#define BLOCK_SIZE 16        /* Minimum allocation size */

/* Memory block header */
struct mem_block {
    size_t size;
    bool free;
    struct mem_block* next;
    struct mem_block* prev;
};

static struct mem_block* heap_start = NULL;
static bool mm_initialized = false;

/* Initialize memory management */
void mm_init(void) {
    heap_start = (struct mem_block*)HEAP_START;
    heap_start->size = HEAP_SIZE - sizeof(struct mem_block);
    heap_start->free = true;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    mm_initialized = true;
}

/* Find a free block of at least the given size */
static struct mem_block* find_free_block(size_t size) {
    struct mem_block* current = heap_start;
    
    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

/* Split a block if it's larger than needed */
static void split_block(struct mem_block* block, size_t size) {
    if (block->size > size + sizeof(struct mem_block) + BLOCK_SIZE) {
        struct mem_block* new_block = (struct mem_block*)((char*)block + sizeof(struct mem_block) + size);
        new_block->size = block->size - size - sizeof(struct mem_block);
        new_block->free = true;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next) {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = size;
    }
}

/* Merge adjacent free blocks */
static void merge_free_blocks(struct mem_block* block) {
    /* Merge with next block */
    if (block->next && block->next->free) {
        block->size += block->next->size + sizeof(struct mem_block);
        if (block->next->next) {
            block->next->next->prev = block;
        }
        block->next = block->next->next;
    }
    
    /* Merge with previous block */
    if (block->prev && block->prev->free) {
        block->prev->size += block->size + sizeof(struct mem_block);
        if (block->next) {
            block->next->prev = block->prev;
        }
        block->prev->next = block->next;
    }
}

/* Allocate memory */
void* kmalloc(size_t size) {
    if (!mm_initialized || size == 0) {
        return NULL;
    }
    
    /* Align size to BLOCK_SIZE */
    size = (size + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);
    
    struct mem_block* block = find_free_block(size);
    if (!block) {
        return NULL; /* Out of memory */
    }
    
    split_block(block, size);
    block->free = false;
    
    return (char*)block + sizeof(struct mem_block);
}

/* Free memory */
void kfree(void* ptr) {
    if (!ptr || !mm_initialized) {
        return;
    }
    
    struct mem_block* block = (struct mem_block*)((char*)ptr - sizeof(struct mem_block));
    block->free = true;
    
    merge_free_blocks(block);
}

/* Get memory statistics */
void get_memory_stats(size_t* total, size_t* used, size_t* free) {
    *total = HEAP_SIZE;
    *used = 0;
    *free = 0;
    
    if (!mm_initialized) {
        return;
    }
    
    struct mem_block* current = heap_start;
    while (current) {
        if (current->free) {
            *free += current->size;
        } else {
            *used += current->size;
        }
        current = current->next;
    }
}
