#include "kronos.h"
#include "multiboot2.h"

/* Kronos OS Kernel Main Entry Point */

static u64 boot_time = 0;

void kernel_main(struct multiboot2_info* mbi) {
    /* Initialize VGA text mode first for output */
    vga_init();
    vga_clear();
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    
    vga_puts("Kronos OS v1.0 - 64-bit Operating System\n");
    vga_puts("========================================\n\n");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Initializing kernel subsystems...\n");
    
    /* Initialize Global Descriptor Table */
    vga_puts("Setting up GDT... ");
    gdt_init();
    vga_puts("OK\n");
    
    /* Initialize Interrupt Descriptor Table */
    vga_puts("Setting up IDT... ");
    idt_init();
    vga_puts("OK\n");
    
    /* Install IRQ handlers */
    vga_puts("Installing IRQ handlers... ");
    irq_install();
    vga_puts("OK\n");
    
    /* Initialize memory management */
    vga_puts("Initializing memory management... ");
    mm_init();
    vga_puts("OK\n");
    
    /* Initialize keyboard driver */
    vga_puts("Initializing keyboard driver... ");
    keyboard_init();
    vga_puts("OK\n");
    
    /* Initialize shell */
    vga_puts("Initializing shell... ");
    shell_init();
    vga_puts("OK\n");
    
    vga_puts("\nKernel initialization complete!\n");
    vga_puts("Type 'help' for available commands.\n\n");
    
    /* Record boot time */
    boot_time = 0; // Will be updated when timer is implemented
    
    /* Start the shell */
    shell_run();
    
    /* Should never reach here */
    vga_puts("Kernel exited unexpectedly!\n");
    system_halt();
}

/* Get system uptime in seconds */
u64 get_uptime(void) {
    // TODO: Implement proper timer-based uptime
    return 0;
}

/* System halt function */
void system_halt(void) {
    vga_puts("System halted. You can safely power off.\n");
    __asm__ volatile ("cli; hlt");
    while(1) {
        __asm__ volatile ("hlt");
    }
}

/* System reboot function */
void system_reboot(void) {
    vga_puts("Rebooting system...\n");
    
    /* Try keyboard controller reset */
    __asm__ volatile ("cli");
    
    u8 temp;
    do {
        temp = inb(0x64);
        if (temp & 0x01) {
            inb(0x60);
        }
    } while (temp & 0x02);
    
    outb(0x64, 0xFE);
    
    /* If that fails, try triple fault */
    __asm__ volatile ("int $0x00");
    
    /* Last resort - infinite loop */
    system_halt();
}


