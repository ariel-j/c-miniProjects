section .data                               ; Data section
    buffer dd 0                             ; Single byte buffer for reading chars
    newline dd   10                           ; Newline character (ASCII 10)
    error_msg db "Cannot open file", 10     ; Error message with newline
    error_len equ $ - error_msg             ; Length of error message

section .rodata
    out_fmt: db “Argument: %s\n”, 0

section .bss                                ; Uninitialized data section
    infile resd 1                           ; Input file descriptor
    outfile resd 1                          ; Output file descriptor

section .text                               ; Code section
    global _start                           ; Entry point
    global main
    extern strlen

print:


main:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]   ; argc
    mov ecx, [ebp+12]  ; argv

Next:
    pushad
    push dword [edx] ; push av[i] (i=0 first)
    push dword out_fmt
    call print ; printf(out_fmt, [edx])
    add esp, 8 ; “remove” printf arguments
    popad 
    add edx, 4 ; advance edx to &av[i+1]
    dec ecx ; dec. arg counter
    jnz Next ; loop if not yet zero
    mov esp, ebp
    pop ebp
    ret
