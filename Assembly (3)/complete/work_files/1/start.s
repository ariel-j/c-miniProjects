section .data                               ; Data section
    buffer db 0                             ; Single byte buffer for reading chars
    newline db 10                           ; Newline character (ASCII 10)
    error_msg db "Cannot open file", 10     ; Error message with newline
    error_len equ $ - error_msg             ; Length of error message

section .bss                                ; Uninitialized data section
    infile resd 1                           ; Input file descriptor
    outfile resd 1                          ; Output file descriptor

section .text                               ; Code section
    global _start                           ; Entry point
    extern strlen                           ; External function from util

_start:
    ; Store command line arguments
    pop dword ecx                           ; Get argc which is 32 bit thus double word
    mov esi, esp                            ; Get argv into esi
    
    ; Set default values
    mov dword [infile], 0                   ; stdin by default
    mov dword [outfile], 1                  ; stdout by default
    
    ; Parse arguments
    dec ecx                                 ; Skip program name (dec = decrease -> argc - 1)
    add esi, 4                              ; Point to first actual argument (add 4 -> moving to the first)
    
parse_args:
    cmp ecx, 0                              ; (argc == 0 or null) 
    jle start_encoding                      ; If no more args, start encoding
    
    mov edx, [esi]                          ; (else) Get current argument
    
    ; Print arg (1.A)
    push ecx                                ; Save argc
    push esi                                ; Save argv
    
    push edx                                ; Push arg for strlen
    call strlen                             ; Get length of argument
    add esp, 4                              ; Clean up strlen parameter
    mov edx, eax                            ; Length for write
    
    mov eax, 4                              ; sys_write
    mov ebx, 2                              ; stderr
    mov ecx, [esi]                          ; get the value that esi is pointing at into ecx
    int 0x80                                ; write syscall
    
    ; Print newline
    mov eax, 4                              ; sys_write
    mov ebx, 2                              ; stderr
    mov ecx, newline                        ; add newline char into ecx
    mov edx, 1                              ; length = 1
    int 0x80                                ; write syscall (it is in other words = EXECUTE)
    
    pop esi                                 ; Restore registers 
    pop ecx                                 ; 
    
    ; Check for -i flag
    mov edx, [esi]                          ; Get argument agai
    cmp byte [edx], '-'                     ; Check first char (argv[i][0] == '-')
    jne not_input                           ; jump to not_input if not equal
    cmp byte [edx + 1], 'i'                 ; Check second char (argv[i][1] == 'i')
    jne not_input                           ; jump to not_input if not equal
    
    ; Open input file
    mov eax, 5                              ; sys_open
    lea ebx, [edx + 2]                      ; filename (skip -i)
    xor ecx, ecx                            ; O_RDONLY
    mov edx, 0644o                          ; File permissions: owner can read/write (6), group can read (4), others can read (4)
    int 0x80                                ; syscall
    
    cmp eax, 0                              ; Check if open failed 
    jl open_error                           ; jump if equal to open_error
    mov [infile], eax                       ; Store file descriptor
    jmp next_arg                            ; jump to next_arg

not_input:
    ; Check for -o flag
    mov edx, [esi]                          ; Get current argument pointer
    cmp byte [edx], '-'                     ; Check first char
    jne next_arg
    cmp byte [edx + 1], 'o'                 ; Check second char
    jne next_arg
    
    ; At this point, we know it's a -o argument
    push ecx                                ; Save our counters
    push esi
    
    mov eax, 5                              ; sys_open
    mov ebx, edx                            ; Get argument string
    add ebx, 2                              ; Skip past "-o"
    mov ecx, 0x241                          ; O_WRONLY | O_CREAT | O_TRUNC
    mov edx, 0644o                          ; File permissions
    int 0x80
    
    pop esi                                 ; Restore our counters
    pop ecx
    
    cmp eax, 0                              ; Check if open failed
    jl open_error
    mov [outfile], eax                      ; Store file descriptor
    jmp next_arg


next_arg:
    add esi, 4                              ; Next argument
    dec ecx                                 ; Decrease count
    jmp parse_args                          

open_error:
    ; Print error message
    mov eax, 4                              ; sys_write
    mov ebx, 2                              ; stderr
    mov ecx, error_msg                      ; store in ecx error_msg 
    mov edx, error_len                      ; store in edx error len 
    int 0x80                                ; execute meaning -> write into stderr the error_msg with this length

    mov ebx, 0x55                           ; Exit code 0x55 
    jmp exit

start_encoding:
    ; Main encoding loop
read_char:
    mov eax, 3                              ; sys_read
    mov ebx, [infile]                       ; Input file descriptor
    mov ecx, buffer                         ; Read into buffer
    mov edx, 1                              ; Read 1 byte
    int 0x80                                ; syscall -> do all we have in eax,ebx,ecx,edx.
    
    cmp eax, 0                              ; Check for EOF
    jle cleanup                             ; If EOF or error, exit
    
    ; Check if char should be encoded
    mov al, [buffer]                        ; Get char from buffer
    cmp al, 'A'
    jl write_char                           ; If < 'A', don't encode
    cmp al, 'z'
    jg write_char                           ; If > 'z', don't encode
    
    ; Encode the character
    inc al                                  ; Add 1 to character
    mov [buffer], al                        ; Store back in buffer
    
write_char:
    mov eax, 4                              ; sys_write
    mov ebx, [outfile]                      ; Output file descriptor
    mov ecx, buffer                         ; Write from buffer
    mov edx, 1                              ; Write 1 byte
    int 0x80
    
    ; Add debug check
    cmp eax, 0                              ; Check if write failed
    jl write_error                          ; Jump if error
    
    jmp read_char                           ; Continue loop

write_error:
    ; Print error message
    mov eax, 4                              ; sys_write
    mov ebx, 2                              ; stderr
    mov ecx, error_msg          
    mov edx, error_len
    int 0x80

cleanup:
    ; Close files if not stdin/stdout
    mov ebx, [infile]                       ; copy the input descriptor into ebx
    cmp ebx, 0                              ; Check if not stdin
    je check_outfile                        ; if equal -> check_outfile
    mov eax, 6                              ; sys_close 
    int 0x80                                ; syscall
    
check_outfile:
    mov ebx, [outfile]
    cmp ebx, 1                              ; Check if not stdout
    je exit
    mov eax, 6                              ; sys_close
    int 0x80
    
exit:
    mov eax, 1                              ; sys_exit
    int 0x80