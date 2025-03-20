section .data
    msg db 'Hello, Infected File', 10   ; Message with newline
    msg_len equ $ - msg                 ; Length of message

section .text
    global code_start
    global code_end
    global infection
    global infector

code_start:
infection:
    push ebp
    mov ebp, esp

    ; Print infection message using single syscall
    mov eax, 4              ; sys_write
    mov ebx, 1              ; stdout
    mov ecx, msg            ; message
    mov edx, msg_len        ; length
    int 0x80

    mov esp, ebp
    pop ebp
    ret

infector:
    push ebp
    mov ebp, esp
    
    ; Open file for append
    mov eax, 5              ; sys_open
    mov ebx, [ebp+8]        ; filename from parameter
    mov ecx, 0x441          ; O_WRONLY | O_APPEND
    mov edx, 0644o          ; mode
    int 0x80
    
    ; Check if open failed
    cmp eax, 0
    jl infection_failed
    mov ebx, eax            ; save fd for writing
    
    ; Write infection message first
    push ebx                ; save fd
    mov eax, 4              ; sys_write
    ; ebx already has fd
    mov ecx, msg            ; write message
    mov edx, msg_len        ; message length
    int 0x80
    
    pop ebx                 ; restore fd
    
    ; Close file
    mov eax, 6              ; sys_close
    int 0x80
    
    mov eax, 1             ; Return success
    jmp done_infector
    
infection_failed:
    mov eax, 0             ; Return failure
    
done_infector:
    mov esp, ebp
    pop ebp
    ret

code_end:
    nop                     ; Ensure code_end has its own address