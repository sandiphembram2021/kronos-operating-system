#include "kronos.h"

/* Interrupt Descriptor Table for 64-bit mode */

struct idt_entry {
    u16 base_low;
    u16 selector;
    u8  ist;
    u8  flags;
    u16 base_middle;
    u32 base_high;
    u32 reserved;
} __attribute__((packed));

struct idt_ptr {
    u16 limit;
    u64 base;
} __attribute__((packed));

/* IDT entries */
static struct idt_entry idt[256];
static struct idt_ptr idt_pointer;

/* External assembly functions */
extern void idt_flush(u64 idt_ptr);
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

/* Set an IDT entry */
static void idt_set_gate(u8 num, u64 base, u16 selector, u8 flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_middle = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    
    idt[num].selector = selector;
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}

/* Initialize the Interrupt Descriptor Table */
void idt_init(void) {
    idt_pointer.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_pointer.base = (u64)&idt;
    
    /* Clear IDT */
    memset(&idt, 0, sizeof(struct idt_entry) * 256);
    
    /* Install ISRs (Interrupt Service Routines) */
    idt_set_gate(0, (u64)isr0, 0x08, 0x8E);
    idt_set_gate(1, (u64)isr1, 0x08, 0x8E);
    idt_set_gate(2, (u64)isr2, 0x08, 0x8E);
    idt_set_gate(3, (u64)isr3, 0x08, 0x8E);
    idt_set_gate(4, (u64)isr4, 0x08, 0x8E);
    idt_set_gate(5, (u64)isr5, 0x08, 0x8E);
    idt_set_gate(6, (u64)isr6, 0x08, 0x8E);
    idt_set_gate(7, (u64)isr7, 0x08, 0x8E);
    idt_set_gate(8, (u64)isr8, 0x08, 0x8E);
    idt_set_gate(9, (u64)isr9, 0x08, 0x8E);
    idt_set_gate(10, (u64)isr10, 0x08, 0x8E);
    idt_set_gate(11, (u64)isr11, 0x08, 0x8E);
    idt_set_gate(12, (u64)isr12, 0x08, 0x8E);
    idt_set_gate(13, (u64)isr13, 0x08, 0x8E);
    idt_set_gate(14, (u64)isr14, 0x08, 0x8E);
    idt_set_gate(15, (u64)isr15, 0x08, 0x8E);
    idt_set_gate(16, (u64)isr16, 0x08, 0x8E);
    idt_set_gate(17, (u64)isr17, 0x08, 0x8E);
    idt_set_gate(18, (u64)isr18, 0x08, 0x8E);
    idt_set_gate(19, (u64)isr19, 0x08, 0x8E);
    idt_set_gate(20, (u64)isr20, 0x08, 0x8E);
    idt_set_gate(21, (u64)isr21, 0x08, 0x8E);
    idt_set_gate(22, (u64)isr22, 0x08, 0x8E);
    idt_set_gate(23, (u64)isr23, 0x08, 0x8E);
    idt_set_gate(24, (u64)isr24, 0x08, 0x8E);
    idt_set_gate(25, (u64)isr25, 0x08, 0x8E);
    idt_set_gate(26, (u64)isr26, 0x08, 0x8E);
    idt_set_gate(27, (u64)isr27, 0x08, 0x8E);
    idt_set_gate(28, (u64)isr28, 0x08, 0x8E);
    idt_set_gate(29, (u64)isr29, 0x08, 0x8E);
    idt_set_gate(30, (u64)isr30, 0x08, 0x8E);
    idt_set_gate(31, (u64)isr31, 0x08, 0x8E);
    
    /* Load the IDT */
    idt_flush((u64)&idt_pointer);
}

/* Install IRQ handlers */
void irq_install(void) {
    /* Remap PIC */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
    
    /* Install IRQ handlers */
    idt_set_gate(32, (u64)irq0, 0x08, 0x8E);  /* Timer */
    idt_set_gate(33, (u64)irq1, 0x08, 0x8E);  /* Keyboard */
    idt_set_gate(34, (u64)irq2, 0x08, 0x8E);
    idt_set_gate(35, (u64)irq3, 0x08, 0x8E);
    idt_set_gate(36, (u64)irq4, 0x08, 0x8E);
    idt_set_gate(37, (u64)irq5, 0x08, 0x8E);
    idt_set_gate(38, (u64)irq6, 0x08, 0x8E);
    idt_set_gate(39, (u64)irq7, 0x08, 0x8E);
    idt_set_gate(40, (u64)irq8, 0x08, 0x8E);
    idt_set_gate(41, (u64)irq9, 0x08, 0x8E);
    idt_set_gate(42, (u64)irq10, 0x08, 0x8E);
    idt_set_gate(43, (u64)irq11, 0x08, 0x8E);
    idt_set_gate(44, (u64)irq12, 0x08, 0x8E);
    idt_set_gate(45, (u64)irq13, 0x08, 0x8E);
    idt_set_gate(46, (u64)irq14, 0x08, 0x8E);
    idt_set_gate(47, (u64)irq15, 0x08, 0x8E);
    
    /* Enable interrupts */
    __asm__ volatile ("sti");
}

/* Interrupt handler structure */
struct interrupt_frame {
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
};

/* ISR handler */
void isr_handler(struct interrupt_frame* frame, u64 interrupt_number) {
    vga_printf("Received interrupt: %d\n", interrupt_number);

    if (interrupt_number == 14) { /* Page fault */
        u64 faulting_address;
        __asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));
        vga_printf("Page fault at address: 0x%x\n", faulting_address);
    }
}

/* IRQ handler */
void irq_handler(struct interrupt_frame* frame, u64 irq_number) {
    /* Send EOI to PIC */
    if (irq_number >= 8) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);

    switch (irq_number) {
        case 0: /* Timer */
            /* Timer tick - could update uptime here */
            break;
        case 1: /* Keyboard */
            keyboard_interrupt_handler();
            break;
        default:
            break;
    }
}
