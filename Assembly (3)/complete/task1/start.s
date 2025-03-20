section .data                               
    buffer db 0                             
    newline db 10                           
    error_msg db "Cannot open file", 10     
    error_len equ $ - error_msg             

section .bss                                
    infile resd 1                           
    outfile resd 1                          

section .text                               
    global main                             
    extern strlen                           

main:
    push ebp                                ; Set up stack frame
    mov ebp, esp

    mov ecx, [ebp+8]                        ; Get argc from stack (first parameter to main)
    mov esi, [ebp+12]                       ; Get argv from stack (second parameter)
    
    mov dword [infile], 0                   ; stdin by default
    mov dword [outfile], 1                  ; stdout by default
    
    dec ecx                                 ; Skip program name
    add esi, 4                              ; Point to first actual argument
    
parse_args:
    cmp ecx, 0                              
    jle start_encoding                      
    
    mov edx, [esi]                          
    
    ; Print arg (1.A)
    push ecx                                
    push esi                                
    
    push edx                                
    call strlen                             
    add esp, 4                              
    mov edx, eax                            
    
    mov eax, 4                              
    mov ebx, 2                              
    mov ecx, [esi]                          
    int 0x80                                
    
    ; Print newline
    mov eax, 4                              
    mov ebx, 2                              
    mov ecx, newline                        
    mov edx, 1                              
    int 0x80                                
    
    pop esi                                 
    pop ecx                                 
    
    ; Check for -i flag
    mov edx, [esi]                          
    cmp byte [edx], '-'                     
    jne not_input                           
    cmp byte [edx + 1], 'i'                 
    jne not_input                           
    
    ; Open input file
    push ecx                                ; Save registers before system call
    push esi

    mov eax, 5                              
    lea ebx, [edx + 2]                      
    xor ecx, ecx                            
    mov edx, 0644o                          
    int 0x80                                
    
    pop esi                                 ; Restore registers
    pop ecx

    cmp eax, 0                              
    jl open_error                           
    mov [infile], eax                       
    jmp next_arg                            

not_input:
    mov edx, [esi]                          
    cmp byte [edx], '-'                     
    jne next_arg
    cmp byte [edx + 1], 'o'                 
    jne next_arg
    
    push ecx                                
    push esi
    
    mov eax, 5                              
    mov ebx, edx                            
    add ebx, 2                              
    mov ecx, 0x241                          
    mov edx, 0644o                          
    int 0x80
    
    pop esi                                 
    pop ecx
    
    cmp eax, 0                              
    jl open_error
    mov [outfile], eax                      
    jmp next_arg

next_arg:
    add esi, 4                              
    dec ecx                                 
    jmp parse_args                          

open_error:
    mov eax, 4                              
    mov ebx, 2                              
    mov ecx, error_msg                      
    mov edx, error_len                      
    int 0x80                                

    mov ebx, 0x55                           
    jmp cleanup_and_exit

start_encoding:
read_char:
    mov eax, 3                              
    mov ebx, [infile]                       
    mov ecx, buffer                         
    mov edx, 1                              
    int 0x80                                
    
    cmp eax, 0                              
    jle cleanup_and_exit                    
    
    mov al, [buffer]                        
    cmp al, 'A'
    jl write_char                           
    cmp al, 'z'
    jg write_char                           
    
    inc al                                  
    mov [buffer], al                        
    
write_char:
    mov eax, 4                              
    mov ebx, [outfile]                      
    mov ecx, buffer                         
    mov edx, 1                              
    int 0x80
    
    cmp eax, 0                              
    jl write_error                          
    
    jmp read_char                           

write_error:
    mov eax, 4                              
    mov ebx, 2                              
    mov ecx, error_msg          
    mov edx, error_len
    int 0x80

cleanup_and_exit:
    ; Close files if not stdin/stdout
    mov ebx, [infile]                       
    cmp ebx, 0                              
    je check_outfile                        
    mov eax, 6                              
    int 0x80                                
    
check_outfile:
    mov ebx, [outfile]
    cmp ebx, 1                              
    je exit
    mov eax, 6                              
    int 0x80
    
exit:
    mov esp, ebp                            ; Restore stack frame
    pop ebp
    xor eax, eax                            ; Return 0
    ret

section .text
global _start
_start:
    pop    dword ecx                        ; ecx = argc
    mov    esi, esp                         ; esi = argv
    mov    eax, ecx                         ; put argc into eax
    shl    eax, 2                          ; multiply by 4 to get size
    add    eax, esi                         ; add to argv address
    add    eax, 4                          ; skip NULL at end of argv
    push   dword eax                        ; char *envp[]
    push   dword esi                        ; char* argv[]
    push   dword ecx                        ; int argc

    call    main                            

    mov     ebx, eax                        ; exit code
    mov     eax, 1                          ; sys_exit
    int     0x80