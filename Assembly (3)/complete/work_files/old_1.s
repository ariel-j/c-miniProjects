section .data
    input_fd db 0       ; File descriptor for input (0 = stdin by default) - buffer
    newLine db 10       ; new Line character (ascii 10)
    output_fd db 1      ; File descriptor for output (1 = stdout by default)
    error_msg db "Error opening file", 10
    error_len equ $ - error_msg  ; $ = current location. err msg = starting location

section .bss
    buffer resb 1       ; Buffer for reading characters
    
section .text
    global _start
    global main            ; Make main visible to the linker

; System call numbers
SYS_READ equ 3
SYS_WRITE equ 4
SYS_OPEN equ 5
SYS_CLOSE equ 6
SYS_EXIT equ 1

main:
    ; Save argc and argv
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]   ; argc
    mov ecx, [ebp+12]  ; argv

    ; Print arguments for debug (Task 1.A)
    push ebx           ; Save argc
    push ecx           ; Save argv
    call print_args
    pop ecx            ; Restore argv
    pop ebx            ; Restore argc

    ; Parse command line arguments
    dec ebx            ; Skip program name
    add ecx, 4         ; Point to first argument
    
parse_args:
    test ebx, ebx
    jz encode_loop     ; If no more args, start encoding
    push ebx           ; Save counter
    push ecx           ; Save argv pointer
    
    mov edx, [ecx]     ; Get current argument
    cmp byte [edx], '-'
    jne next_arg
    
    cmp byte [edx+1], 'i'
    je handle_input
    cmp byte [edx+1], 'o'
    je handle_output
    
next_arg:
    pop ecx
    pop ebx
    add ecx, 4         ; Next argument
    dec ebx
    jmp parse_args

handle_input:
    add edx, 2         ; Skip "-i"
    mov eax, SYS_OPEN
    mov ebx, edx       ; Filename
    mov ecx, 0         ; O_RDONLY
    mov edx, 0644o     ; File permissions
    int 0x80
    
    cmp eax, 0
    jl error_exit
    
    mov [input_fd], eax
    jmp next_arg

handle_output:
    add edx, 2         ; Skip "-o"
    mov eax, SYS_OPEN
    mov ebx, edx       ; Filename
    mov ecx, 0x241     ; O_WRONLY | O_CREAT | O_TRUNC
    mov edx, 0644o     ; File permissions
    int 0x80
    
    cmp eax, 0
    jl error_exit
    
    mov [output_fd], eax
    jmp next_arg

encode_loop:
    ; Read one character
    mov eax, SYS_READ
    mov ebx, [input_fd]
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
    mov ebx, [output_fd]
    mov ecx, buffer
    mov edx, 1
    int 0x80
    jmp encode_loop

cleanup:
    ; Close files if they were opened
    mov eax, [input_fd]
    cmp eax, 0
    je check_output
    mov eax, SYS_CLOSE
    mov ebx, [input_fd]
    int 0x80
    
check_output:
    mov eax, [output_fd]
    cmp eax, 1
    je exit_normal
    mov eax, SYS_CLOSE
    mov ebx, [output_fd]
    int 0x80

exit_normal:
    mov eax, 0         ; Return 0
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

; Function to print arguments (Task 1.A)
print_args:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+12]  ; argv
    mov ecx, [ebp+8]   ; argc
    
print_loop:
    test ecx, ecx
    jz print_done
    push ecx
    push ebx
    
    mov edx, [ebx]     ; Get current argument
    mov ecx, edx       ; Calculate string length
    xor eax, eax
strlen_loop:
    cmp byte [ecx], 0
    je strlen_done
    inc eax
    inc ecx
    jmp strlen_loop
strlen_done:
    
    mov ecx, [ebx]     ; Reset pointer to string start
    push eax           ; Save length
    
    mov eax, SYS_WRITE
    mov ebx, 1         ; stdout
    push edx
    mov edx, [esp+4]   ; Length
    int 0x80
    
    mov eax, SYS_WRITE
    mov ebx, 1         ; stdout
    mov ecx, newline
    mov edx, 1
    int 0x80
    
    pop edx
    pop eax
    pop ebx
    pop ecx
    add ebx, 4         ; Next argument
    dec ecx
    jmp print_loop
    
print_done:
    mov esp, ebp
    pop ebp
    ret

section .data
    newline db 10      ; Newline character