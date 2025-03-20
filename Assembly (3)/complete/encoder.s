section .data
    input_fd db 0       ; File descriptor for input (0 = stdin by default)
    output_fd db 1      ; File descriptor for output (1 = stdout by default)
    error_msg db "Error opening file", 10
    error_len equ $ - error_msg
    newline db 10      ; Newline character

section .bss
    buffer resb 1       ; Buffer for reading characters

section .text
    global main        ; Only export main if using gcc

; System call numbers
SYS_READ equ 3
SYS_WRITE equ 4
SYS_OPEN equ 5
SYS_CLOSE equ 6
SYS_EXIT equ 1

main:
    push ebp
    mov ebp, esp
    
    ; Get argc and argv
    mov ebx, [ebp+8]   ; argc
    mov ecx, [ebp+12]  ; argv

    ; Save original argv and argc for later
    push ecx           ; Save argv
    push ebx           ; Save argc

    ; Print arguments for debug
    call print_args

    ; Restore argc and argv
    pop ebx            ; Restore argc
    pop ecx            ; Restore argv

    ; Parse command line arguments
    dec ebx            ; Skip program name
    add ecx, 4         ; Point to first argument
    
parse_args:
    test ebx, ebx
    jz start_encode    ; If no more args, start encoding
    
    mov edx, [ecx]     ; Get current argument
    cmp byte [edx], '-'
    jne next_arg
    
    cmp byte [edx+1], 'i'
    je handle_input
    cmp byte [edx+1], 'o'
    je handle_output
    
next_arg:
    add ecx, 4         ; Next argument
    dec ebx
    jmp parse_args

handle_input:
    add edx, 2         ; Skip "-i"
    push ecx           ; Save argv pointer
    push ebx           ; Save argc
    
    mov eax, SYS_OPEN
    mov ebx, edx       ; Filename
    mov ecx, 0         ; O_RDONLY
    mov edx, 0644o     ; File permissions
    int 0x80
    
    pop ebx            ; Restore argc
    pop ecx            ; Restore argv
    
    cmp eax, 0
    jl error_exit
    
    mov [input_fd], al
    jmp next_arg

handle_output:
    add edx, 2         ; Skip "-o"
    push ecx           ; Save argv pointer
    push ebx           ; Save argc
    
    mov eax, SYS_OPEN
    mov ebx, edx       ; Filename
    mov ecx, 0x241     ; O_WRONLY | O_CREAT | O_TRUNC
    mov edx, 0644o     ; File permissions
    int 0x80
    
    pop ebx            ; Restore argc
    pop ecx            ; Restore argv
    
    cmp eax, 0
    jl error_exit
    
    mov [output_fd], al
    jmp next_arg

start_encode:          ; New label to clearly separate encoding logic
    ; Reset file descriptors if none were specified
    cmp byte [input_fd], 0
    jne encode_loop
    mov byte [input_fd], 0    ; Use stdin
    
    cmp byte [output_fd], 1
    jne encode_loop
    mov byte [output_fd], 1   ; Use stdout

encode_loop:
    ; Read one character
    mov eax, SYS_READ
    movzx ebx, byte [input_fd]
    mov ecx, buffer
    mov edx, 1
    int 0x80
    
    ; Check for EOF or error
    cmp eax, 0
    jle cleanup
    
    ; Encode character if it's a letter
    mov al, [buffer]
    cmp al, 'A'
    jl write_char
    cmp al, 'z'
    jg write_char
    inc al
    mov [buffer], al
    
write_char:
    mov eax, SYS_WRITE
    movzx ebx, byte [output_fd]
    mov ecx, buffer
    mov edx, 1
    int 0x80
    jmp encode_loop

cleanup:
    ; Close input file if not stdin
    movzx eax, byte [input_fd]
    cmp eax, 0
    je check_output
    mov eax, SYS_CLOSE
    movzx ebx, byte [input_fd]
    int 0x80
    
check_output:
    ; Close output file if not stdout
    movzx eax, byte [output_fd]
    cmp eax, 1
    je exit_normal
    mov eax, SYS_CLOSE
    movzx ebx, byte [output_fd]
    int 0x80

exit_normal:
    xor eax, eax       ; Return 0
    mov esp, ebp
    pop ebp
    ret

error_exit:
    mov eax, SYS_WRITE
    mov ebx, 2         ; stderr
    mov ecx, error_msg
    mov edx, error_len
    int 0x80
    
    mov eax, 1         ; Return 1 on error
    mov esp, ebp
    pop ebp
    ret

print_args:
    push ebp
    mov ebp, esp
    push ebx           ; Save callee-saved registers
    push esi
    
    mov ebx, [ebp+8]   ; argc
    mov esi, [ebp+12]  ; argv
    
print_loop:
    test ebx, ebx
    jz print_done
    
    mov edx, [esi]     ; Get current argument string
    
    ; Calculate string length
    push ebx           ; Save counter
    push esi           ; Save pointer
    mov ecx, edx       ; String to measure
    xor eax, eax       ; Length counter
strlen_loop:
    cmp byte [ecx], 0
    je strlen_done
    inc eax
    inc ecx
    jmp strlen_loop
strlen_done:
    
    ; Print the argument
    push eax           ; Save length
    mov eax, SYS_WRITE
    mov ebx, 1         ; stdout
    mov ecx, edx       ; String to print
    mov edx, [esp]     ; Length
    int 0x80
    
    ; Print newline
    mov eax, SYS_WRITE
    mov ebx, 1
    mov ecx, newline
    mov edx, 1
    int 0x80
    
    pop eax            ; Clean up length
    pop esi            ; Restore pointer
    pop ebx            ; Restore counter
    
    add esi, 4         ; Next argument
    dec ebx
    jmp print_loop
    
print_done:
    pop esi            ; Restore callee-saved registers
    pop ebx
    mov esp, ebp
    pop ebp
    ret