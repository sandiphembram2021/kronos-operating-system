; GDT flush function for 64-bit mode

global gdt_flush

section .text
bits 64

gdt_flush:
    lgdt [rdi]          ; Load GDT pointer
    
    ; Reload segment registers
    mov ax, 0x10        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump to reload CS
    push 0x08           ; Code segment selector
    lea rax, [rel .reload_cs]
    push rax
    retfq
    
.reload_cs:
    ret
