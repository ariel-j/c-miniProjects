section .data
    msg db 'Hello, World!', 0xa  ; String to print, 0xa is newline
    len equ $ - msg              ; Length of the string

section .text
global _start

_start:
    ; Write the message to stdout
    mov eax, 4          ; System call number for sys_write
    mov ebx, 1          ; File descriptor 1 is stdout
    mov ecx, msg        ; Pointer to message to write
    mov edx, len        ; Message length
    int 0x80            ; Make system call

    ; Exit program
    mov eax, 1          ; System call number for sys_exit
    xor ebx, ebx        ; Return 0 status
    int 0x80            ; Make system call