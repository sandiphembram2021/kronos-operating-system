; Context switching for Kronos OS multitasking
; x86_64 assembly for saving and restoring CPU state

global context_switch
global save_context
global restore_context

section .text
bits 64

; void context_switch(struct process* prev, struct process* next)
; RDI = prev process, RSI = next process
context_switch:
    ; Save current process context if prev is not NULL
    test rdi, rdi
    jz .restore_next
    
    ; Save all general-purpose registers to prev->context
    mov [rdi + 0], rax      ; context.rax
    mov [rdi + 8], rbx      ; context.rbx
    mov [rdi + 16], rcx     ; context.rcx
    mov [rdi + 24], rdx     ; context.rdx
    mov [rdi + 32], rsi     ; context.rsi
    mov [rdi + 40], rdi     ; context.rdi (save original)
    mov [rdi + 48], rbp     ; context.rbp
    mov [rdi + 56], rsp     ; context.rsp
    mov [rdi + 64], r8      ; context.r8
    mov [rdi + 72], r9      ; context.r9
    mov [rdi + 80], r10     ; context.r10
    mov [rdi + 88], r11     ; context.r11
    mov [rdi + 96], r12     ; context.r12
    mov [rdi + 104], r13    ; context.r13
    mov [rdi + 112], r14    ; context.r14
    mov [rdi + 120], r15    ; context.r15
    
    ; Save RIP (return address)
    mov rax, [rsp]
    mov [rdi + 128], rax    ; context.rip
    
    ; Save RFLAGS
    pushfq
    pop rax
    mov [rdi + 136], rax    ; context.rflags
    
    ; Save CR3 (page directory)
    mov rax, cr3
    mov [rdi + 144], rax    ; context.cr3

.restore_next:
    ; Restore next process context
    ; RSI = next process
    
    ; Switch page directory
    mov rax, [rsi + 144]    ; next->context.cr3
    mov cr3, rax
    
    ; Restore RFLAGS
    mov rax, [rsi + 136]    ; next->context.rflags
    push rax
    popfq
    
    ; Restore general-purpose registers
    mov rax, [rsi + 0]      ; next->context.rax
    mov rbx, [rsi + 8]      ; next->context.rbx
    mov rcx, [rsi + 16]     ; next->context.rcx
    mov rdx, [rsi + 24]     ; next->context.rdx
    mov rbp, [rsi + 48]     ; next->context.rbp
    mov rsp, [rsi + 56]     ; next->context.rsp
    mov r8, [rsi + 64]      ; next->context.r8
    mov r9, [rsi + 72]      ; next->context.r9
    mov r10, [rsi + 80]     ; next->context.r10
    mov r11, [rsi + 88]     ; next->context.r11
    mov r12, [rsi + 96]     ; next->context.r12
    mov r13, [rsi + 104]    ; next->context.r13
    mov r14, [rsi + 112]    ; next->context.r14
    mov r15, [rsi + 120]    ; next->context.r15
    
    ; Restore RDI and RSI last
    mov rdi, [rsi + 40]     ; next->context.rdi
    push qword [rsi + 128]  ; Push next->context.rip for return
    mov rsi, [rsi + 32]     ; next->context.rsi
    
    ; Jump to new process
    ret

; Save current CPU context to buffer
; RDI = context buffer
save_context:
    mov [rdi + 0], rax
    mov [rdi + 8], rbx
    mov [rdi + 16], rcx
    mov [rdi + 24], rdx
    mov [rdi + 32], rsi
    mov [rdi + 40], rdi
    mov [rdi + 48], rbp
    mov [rdi + 56], rsp
    mov [rdi + 64], r8
    mov [rdi + 72], r9
    mov [rdi + 80], r10
    mov [rdi + 88], r11
    mov [rdi + 96], r12
    mov [rdi + 104], r13
    mov [rdi + 112], r14
    mov [rdi + 120], r15
    
    ; Save return address
    mov rax, [rsp]
    mov [rdi + 128], rax
    
    ; Save flags
    pushfq
    pop rax
    mov [rdi + 136], rax
    
    ; Save CR3
    mov rax, cr3
    mov [rdi + 144], rax
    
    ret

; Restore CPU context from buffer
; RDI = context buffer
restore_context:
    ; Restore CR3
    mov rax, [rdi + 144]
    mov cr3, rax
    
    ; Restore flags
    mov rax, [rdi + 136]
    push rax
    popfq
    
    ; Restore registers
    mov rax, [rdi + 0]
    mov rbx, [rdi + 8]
    mov rcx, [rdi + 16]
    mov rdx, [rdi + 24]
    mov rbp, [rdi + 48]
    mov rsp, [rdi + 56]
    mov r8, [rdi + 64]
    mov r9, [rdi + 72]
    mov r10, [rdi + 80]
    mov r11, [rdi + 88]
    mov r12, [rdi + 96]
    mov r13, [rdi + 104]
    mov r14, [rdi + 112]
    mov r15, [rdi + 120]
    
    ; Restore RDI and RSI, jump to saved RIP
    push qword [rdi + 128]  ; Push saved RIP
    mov rsi, [rdi + 32]
    mov rdi, [rdi + 40]
    
    ret  ; Jump to saved RIP

; Fork system call implementation
global sys_fork
sys_fork:
    ; Save parent context
    push rbp
    mov rbp, rsp
    
    ; Get current process
    call get_current_process
    test rax, rax
    jz .fork_error
    
    ; Create child process (copy of parent)
    mov rdi, rax
    call process_fork
    
    ; Return value:
    ; Parent: child PID in RAX
    ; Child: 0 in RAX
    
    pop rbp
    ret

.fork_error:
    mov rax, -1
    pop rbp
    ret

; Execute new program (execve)
global sys_execve
sys_execve:
    ; RDI = filename, RSI = argv, RDX = envp
    push rbp
    mov rbp, rsp
    
    ; Load new program
    call load_program
    test rax, rax
    jz .exec_error
    
    ; Replace current process image
    call replace_process_image
    
    ; Should not return if successful
    
.exec_error:
    mov rax, -1
    pop rbp
    ret

; Process exit
global sys_exit
sys_exit:
    ; RDI = exit code
    call process_exit
    ; Should not return
    
    ; If we somehow get here, halt
    cli
    hlt

; Wait for child process
global sys_wait
sys_wait:
    ; RDI = status pointer
    push rbp
    mov rbp, rsp
    
    call process_wait
    
    pop rbp
    ret

; Get process ID
global sys_getpid
sys_getpid:
    call get_current_process
    test rax, rax
    jz .getpid_error
    
    mov eax, [rax + 0]  ; Return PID
    ret

.getpid_error:
    mov rax, -1
    ret

; Get parent process ID
global sys_getppid
sys_getppid:
    call get_current_process
    test rax, rax
    jz .getppid_error
    
    mov eax, [rax + 4]  ; Return PPID
    ret

.getppid_error:
    mov rax, -1
    ret
