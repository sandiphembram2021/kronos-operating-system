; Kronos OS Multiboot2 Header
; This file contains the multiboot2 header required by GRUB2

section .multiboot
align 8

; Multiboot2 header
multiboot_header_start:
    dd 0x36d76289                ; magic number (multiboot2)
    dd 0                         ; architecture (i386)
    dd multiboot_header_end - multiboot_header_start  ; header length
    ; checksum
    dd -(0x36d76289 + 0 + (multiboot_header_end - multiboot_header_start))

    ; Information request tag
    dw 1                         ; type = information request
    dw 0                         ; flags
    dd 12                        ; size
    dd 4                         ; memory map
    
    ; End tag
    dw 0                         ; type = end
    dw 0                         ; flags  
    dd 8                         ; size

multiboot_header_end:
