#include "kronos.h"

/* Global Descriptor Table for 64-bit mode */

struct gdt_entry {
    u16 limit_low;
    u16 base_low;
    u8  base_middle;
    u8  access;
    u8  granularity;
    u8  base_high;
} __attribute__((packed));

struct gdt_ptr {
    u16 limit;
    u64 base;
} __attribute__((packed));

/* GDT entries */
static struct gdt_entry gdt[5];
static struct gdt_ptr gdt_pointer;

/* External assembly function to load GDT */
extern void gdt_flush(u64 gdt_ptr);

/* Set a GDT entry */
static void gdt_set_gate(i32 num, u64 base, u64 limit, u8 access, u8 gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

/* Initialize the Global Descriptor Table */
void gdt_init(void) {
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_pointer.base  = (u64)&gdt;
    
    /* NULL descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);
    
    /* Kernel code segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xAF);
    
    /* Kernel data segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xAF);
    
    /* User code segment */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xAF);
    
    /* User data segment */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xAF);
    
    /* Load the GDT */
    gdt_flush((u64)&gdt_pointer);
}
