/* Kronos OS Linker Script for 64-bit ELF kernel */

ENTRY(start)

SECTIONS
{
    /* Kernel starts at 1MB physical address */
    . = 1M;

    /* Multiboot header must be early in the file */
    .multiboot :
    {
        *(.multiboot)
    }

    /* Text section for code */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    /* Read-only data */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    /* The compiler may produce other sections, by default it will put them in
       a segment with the same name. Simply add stuff here as needed. */
}
