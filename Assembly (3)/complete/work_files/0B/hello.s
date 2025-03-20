section .data
    msg db 'Hello JSON', 10 ; 10 = newline
    len equ $ - msg ; length of msg

section .text
    global _start

_start:

; call write(1,msg,len)
    mov eax, 4   ; SYS_WRITE
    mov ebx, 1   ; STDOUT
    mov ecx, msg ; adress of string to write
    mov edx, len ; length of string to write
    int 0x80     ; make system call

; call exit(0)
    mov eax, 1 ;calling exit
    mov ebx, 0 ;exit status code = 0
    int 0x80   ; system call