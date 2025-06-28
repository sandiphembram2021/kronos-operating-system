; Kronos OS Boot Stub
; Transitions from 32-bit protected mode to 64-bit long mode

global start
extern kernel_main

section .text
bits 32

start:
    ; Save multiboot information
    mov [multiboot_info_ptr], ebx
    
    ; Set up stack
    mov esp, stack_top
    
    ; Check if we're loaded by a multiboot2-compliant bootloader
    cmp eax, 0x36d76289
    jne .no_multiboot
    
    ; Check for CPUID support
    call check_cpuid
    test eax, eax
    jz .no_cpuid
    
    ; Check for long mode support
    call check_long_mode
    test eax, eax
    jz .no_long_mode
    
    ; Set up paging for long mode
    call setup_page_tables
    call enable_paging
    
    ; Load GDT
    lgdt [gdt64.pointer]
    
    ; Jump to 64-bit code
    jmp gdt64.code:long_mode_start

.no_multiboot:
    mov esi, no_multiboot_msg
    call print_error
    jmp halt

.no_cpuid:
    mov esi, no_cpuid_msg
    call print_error
    jmp halt

.no_long_mode:
    mov esi, no_long_mode_msg
    call print_error
    jmp halt

halt:
    cli
    hlt
    jmp halt

; Check if CPUID is supported
check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    ret

; Check if long mode is supported
check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    
    mov eax, 1
    ret

.no_long_mode:
    xor eax, eax
    ret

; Set up page tables for long mode
setup_page_tables:
    ; Clear page tables
    mov edi, page_table_l4
    mov cr3, edi
    xor eax, eax
    mov ecx, 4096
    rep stosd
    mov edi, cr3
    
    ; Set up page table entries
    mov dword [page_table_l4], page_table_l3 + 0x003
    mov dword [page_table_l3], page_table_l2 + 0x003
    mov dword [page_table_l2], 0x000083  ; 2MB pages, present + writable + huge
    
    ret

; Enable paging and long mode
enable_paging:
    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; Enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    
    ret

; Print error message
print_error:
    mov edi, 0xb8000
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x4f  ; White on red
    stosw
    jmp .loop
.done:
    ret

section .bss
align 4096
page_table_l4:
    resb 4096
page_table_l3:
    resb 4096
page_table_l2:
    resb 4096

stack_bottom:
    resb 16384  ; 16KB stack
stack_top:

multiboot_info_ptr:
    resd 1

section .rodata
no_multiboot_msg: db "ERROR: Not loaded by multiboot2 bootloader", 0
no_cpuid_msg: db "ERROR: CPUID not supported", 0
no_long_mode_msg: db "ERROR: Long mode not supported", 0

; GDT for 64-bit mode
section .rodata
gdt64:
    dq 0  ; null descriptor
.code: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53)  ; code segment
.data: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41)  ; data segment
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .text
bits 64
long_mode_start:
    ; Clear segment registers
    mov ax, gdt64.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack
    mov rsp, stack_top
    
    ; Call kernel main
    mov rdi, [multiboot_info_ptr]
    call kernel_main
    
    ; If kernel returns, halt
    cli
    hlt
