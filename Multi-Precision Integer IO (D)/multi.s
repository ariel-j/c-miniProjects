; ================ DATA STRUCTURES ================
section .data
    ; ===== SYSTEM CONSTANTS =====

    STATE           dw 0xAC07         
    MASK            dw 0xB400        

    ; ===== TEST STRUCTURES =====

    x_struct:       db 5
    x_num:          db 0xaa, 1,2,0x44,0x4f
    y_struct:       db 6
    y_num:          db 0xaa, 1,2,3,0x44,0x4f

    ; ===== ERROR MESSAGES =====

    error_msg:          db "error! wrong argument.", 10, 0
    hex_error_msg:      db "Error: Invalid hex character '%c' found", 10, 0
    hex_exit_msg:       db "Program terminated due to invalid input", 10, 0
    malloc_error_msg:   db "Malloc failed", 10, 0
    fgets_error_msg:    db "Error reading input", 10, 0

    ; ===== FORMAT STRINGS =====

    hex_format:         db "%02hhx", 0
    newLine:            db 10, 0
    prompt              db "Insert hex: ", 0

    ; ===== FLAGS =====

    input_flag          db "-I", 0
    random_flag         db "-R", 0

; ================ BUFFER SPACE ================
section .bss
    input_buffer    resb 601          ; Buffer to store input from fgets (600+1 for null)
    struct1         resb 1            ; Store size of the structure (unsigned char)
    struct1_num     resb 300          ; Store multi-precision number (up to 300 bytes)
    struct2         resb 1            ; Store size of the structure (unsigned char)
    struct2_num     resb 300          ; Store multi-precision number (up to 300 bytes)
    result          resb 257          ; Buffer for random_multi result (max size: 256 bytes + 1 for size)

; ================ SECTION .text ================
section .text
    global main                     
    extern printf, puts, fgets, stdin, malloc, strcmp, exit        

; ================ FLAG HANDLING ================
check_flags:
    mov     edx, [ebx + 4]              ; edx <- argv[1]
    push    edx
    mov     ecx, input_flag
    push    ecx

    call    strcmp
    add     esp, 8

    cmp     eax, 0
    je      input_flag_handler

    mov     ebx, [ebp + 3 * 4]          ; ebx <- argv
    mov     edx, [ebx + 4]              ; edx <- argv[1]
    push    edx
    mov     ecx, random_flag
    push    ecx

    call    strcmp
    add     esp, 8

    cmp     eax, 0
    je      random_flag_handler

error_flag:
    mov     eax, error_msg
    push    eax
    call    puts
    jmp     end_program

random_flag_handler:
    mov     eax, struct1
    push    eax
    call    generate_and_print

cont0:
    mov     eax, struct2
    push    eax
    call    generate_and_print

cont:
    jmp     add_and_print

input_flag_handler:
    mov     eax, struct1
    push    eax
    call    get_and_create_struct
    add     esp, 4

    mov     eax, struct2
    push    eax
    call    get_and_create_struct
    add     esp, 4

    mov     eax, struct1
    push    eax
    call    print_struct

    mov     ebx, struct2
    push    ebx
    call    print_struct

; ================ OUTPUT FUNCTIONS ================
add_and_print:
    call    add_multi
    add     esp, 8

    push    eax
    call    print_struct

    jmp     end_program

print_struct:
    push    ebp                     ; Save base pointer
    mov     ebp, esp                ; Set base pointer to current stack pointer

    mov     eax, [ebp + 2 * 4]      ; eax <- struct
    xor     ecx, ecx
    mov     cl, [eax]
    
    add     eax, ecx
    mov     edx, hex_format

print_loop:
    cmp     cl, 0
    je      end_print_loop

    push    ecx
    push    eax
    xor     ebx, ebx
    mov     bl, [eax]
    push    ebx
    push    edx
    call    printf
    pop     edx
    add     esp, 4
    pop     eax
    pop     ecx

    dec     eax
    dec     cl
    jmp     print_loop

end_print_loop:
    mov     eax, newLine
    push    eax
    call    printf

    mov     esp, ebp
    pop     ebp
    ret

; ================ PROGRAM FLOW FUNCTIONS ================
end_program:
    mov     esp, ebp
    pop     ebp
    ret

; ================ RANDOM NUMBER GENERATION ================

; ================ FUNCTION: generate_random_num ================
; Purpose: Generates a random number using LFSR
; Input: None
; Output: eax - random number
; Modifies: ax, dx, cx
; ================================================
generate_random_num:
    mov     ax, [STATE]           ; Load STATE into AX
    mov     dx, [MASK]            ; Load MASK into DX

    and     ax, dx                ; AX = STATE & MASK

    xor     cx, cx                ; Clear CX (parity accumulator)

parity_loop:
    shr     ax, 1                 ; Shift right AX
    adc     cx, 0                 ; Add carry flag (parity bit) to CX
    test    ax, ax                ; Check if AX is zero
    jnz     parity_loop           ; Repeat if not zero
    
    and     cx, 1                 ; Final parity is in CX (1-bit result)

    mov     ax, [STATE]           ; Reload STATE into AX
    shr     ax, 1                 ; Logical right shift
    shl     cx, 15               ; Move parity bit to MSB position
    or      ax, cx               ; Set MSB based on parity
    mov     [STATE], ax           ; Save updated STATE

    movzx   eax, ax              ; Zero-extend AX into EAX
    ret

; ================ FUNCTION: generate_random_multi ================
; Purpose: Generates a random multi-precision number
; Input: [ebp + 8] - Pointer to structure to fill
; Output: None
; Modifies: Various registers
; ================================================
generate_random_multi:
    push    ebp
    mov     ebp, esp
    push    edi
    push    ebx                   ; Save registers we'll use

    ; Generate random length (non-zero)

get_length:
    call    generate_random_num   
    mov     bl, al                ; Use low byte as length
    test    bl, bl                ; Check if zero
    jz      get_length           ; If zero, try again

    mov     edi, [ebp + 8]        ; Get structure pointer
    mov     [edi], bl             ; Store length
    inc     edi                   ; Point to data area
    
    ; Generate random bytes
    movzx   ecx, bl              ; Use length as counter

generate_bytes:
    push    ecx                   ; Save counter
    call    generate_random_num   ; Get random byte
    mov     [edi], al            ; Store in structure
    inc     edi                   ; Next position
    pop     ecx                   ; Restore counter
    loop    generate_bytes        ; Continue until all bytes generated

    pop     ebx                   ; Restore registers
    pop     edi
    mov     esp, ebp
    pop     ebp
    ret

; ================ FUNCTION: generate_and_print ================
; Purpose: Generates and prints a random multi-precision number
; Input: [ebp + 8] - Pointer to structure to fill
; Output: None
; ================================================
generate_and_print:
    push    ebp
    mov     ebp, esp
    
    push    dword [ebp + 8]      ; Pass structure pointer
    call    generate_random_multi
    call    print_struct
    add     esp, 4
    
    mov     esp, ebp
    pop     ebp
    ret

get_and_create_struct:
    push    ebp
    mov     ebp, esp

    call    get_input          
    mov     eax, [ebp + 4 * 2] 

    push    eax                 
    push    ecx               
    call    string_to_hex
    pop     ecx
    pop     eax

    rcr     cl, 1
    adc     cl, 0
    mov     byte [eax], cl

    mov     esp, ebp
    pop     ebp
    ret

; ================ INPUT HANDLING ================
get_input:
    push    ebp
    mov     ebp, esp

    ; Prompt the user
    push    prompt
    call    puts
    add     esp, 4

    mov     eax, input_buffer
    push    dword [stdin]         
    push    600                    
    push    eax                    
    call    fgets
    add     esp, 12

    test    eax, eax              
    jz      fgets_error

    mov     eax, input_buffer
    xor     ecx, ecx
    xor     edx, edx

check_size:
    mov     dl, [eax]

    cmp     dl, 0
    je      end_check_size
    cmp     dl, 10
    je      end_check_size

    inc     cl
    inc     eax
    jmp     check_size

end_check_size:             
    mov     esp, ebp
    pop     ebp
    ret

string_to_hex:
    push    ebp
    mov     ebp, esp
    
    mov     esi, [ebp + 4 * 3]                        
    inc     esi
    xor     edi, edi
    mov     ecx, [ebp + 4 * 2]                  
    mov     ebx, input_buffer                   

    mov     edi, ecx

    rcr     edi, 1
    jnc     even_size
    ; odd size
    xor     eax, eax
    mov     al, [ebx]
    push    eax
    call    hex_char_to_num
    add     esp, 4
    mov     byte [esi + edi], al

    inc     ebx
    dec     cl

even_size:
    dec     edi

continue_:
    cmp     cl, 0
    jle     end_string_to_hexa

    xor     eax, eax
    mov     al, [ebx]
    push    eax
    call    hex_char_to_num
    add     esp, 4

    mov     dl, al
    xor     eax, eax
    mov     al, [ebx + 1]
    push    eax
    call    hex_char_to_num
    shl     edx, 4
    or      edx, eax

    mov     byte [esi + edi], dl

    add     cl, -2
    add     ebx, 2
    dec     edi
    jmp     continue_

end_string_to_hexa:
    mov     esp, ebp
    pop     ebp
    ret

fgets_error:
    push    fgets_error_msg
    call    puts
    push    1                      ; Exit with error code 1
    call    exit

; =============== HEXA TO NUMBER ===============
hex_char_to_num: 
    push    ebp
    mov     ebp, esp

    mov     eax, [ebp + 2*4]
    cmp     eax, '9'             ; 57 = '9'
    jle     numeric_char
    cmp     eax, 'z'             ; 122 = 'z'
    jg      wrong_char
    cmp     eax, 'a'             ; 97 = 'a'
    jl      wrong_char
    sub     eax, 'a'
    add     eax, 10
    jmp     end_make_hexa_num

numeric_char:
    cmp     eax, '0'             ; 48 = '0'
    jl      wrong_char
    sub     eax, '0'
    jmp     end_make_hexa_num

wrong_char:
    ; Print error message
    push    dword [ebp + 2*4]     ; Push the invalid character
    push    hex_error_msg
    call    printf
    add     esp, 8

    ; Print exit message
    push    hex_exit_msg
    call    puts
    add     esp, 4

    ; Exit program with error code 1
    push    1                      ; Exit code 1
    call    exit                   ; External function to terminate program

end_make_hexa_num:
    mov     esp, ebp
    pop     ebp
    ret


; ================ ARITHMETIC OPERATIONS ================
; ================ FUNCTION: add_multi ================
; Purpose: Adds two multi-precision integers
; Input:
;   - [ebp + 8]: Pointer to first structure
;   - [ebp + 12]: Pointer to second structure
; Output:
;   - eax: Pointer to new structure containing sum
; Modifies: eax, ebx, ecx, edx
; ================================================
add_multi:
    push    ebp
    mov     ebp, esp
    sub     esp, 12               ; local var for new struct pointer

    mov     eax, [ebp + 4 * 2]      ; eax = struct1
    mov     ebx, [ebp + 4 * 3]      ; ebx = struct2

    xor     ecx, ecx
    xor     edx, edx
    mov     cl, [eax]           ; cl = size 1
    mov     dl, [ebx]           ; dl = size 2

    cmp     cl, dl
    jg      s1_bigger
    ; s2 is bigger -> exchange them so the bigger will be in eax size in cl
    xchg    eax, ebx
    xchg    ecx, edx

s1_bigger:                                ; the bigger struct is in eax
    mov     dword [ebp - 4 * 2], ecx      ; save the bigger size locally
    mov     dword [ebp - 4 * 3], edx      ; save the smaller size locally
    mov     esi, ecx
    add     esi, 2
    push    edx
    pushad
    push    esi
    call    malloc
    add     esp, 4                        ; Clean malloc arg immediately

    test    eax, eax                      ; Check if malloc returned NULL
    jz      malloc_error                  ; Jump to error handler if malloc failed

    mov     dword [ebp - 4], eax          ; Store malloc result
    popad                                 ; Restore registers
    pop     edx                           ; Restore edx

    dec     esi                         
    mov     edi, [ebp - 4]                
    mov     ecx, esi
    mov     byte [edi], cl            

    clc

    inc     eax  
    inc     ebx  
    inc     edi  
    xor     ecx, ecx   

sum_loop:
    cmp     edx, 0                      
    je      finish_smaller_array       

    push    eax
    push    ebx

    mov     al, byte [eax]    ; Load byte from larger array into AL
    mov     bl, byte [ebx]    ; Load byte from smaller array into BL

carry:
    add     al, cl            
    mov     ecx, 0
    adc     cl, 0             
    add     al, bl            
    adc     cl, 0             

    ; Store the result in the result array
    mov     byte [edi], al    ; Store the result byte in the result array

    ; Restore the values of EAX and EBX
    pop     ebx
    pop     eax

    ; Increment the pointers to move to the next byte in the arrays
    inc     eax
    inc     ebx
    inc     edi

    ; Decrement the loop counter and continue the loop
    dec     edx
    jmp     sum_loop

finish_smaller_array:
    mov     esi, dword [ebp - 4 * 3]
    mov     edx, [ebp - 8]
    sub     edx, esi                
    
    jz      finish_addition        

process_larger_array:
    push    eax
    mov     al, byte [eax]

carry1:
    add     al, cl              
    mov     ecx, 0              
    adc     cl, 0

    mov     byte [edi], al    ; Store the result byte in the result array

    pop     eax

    inc     eax
    inc     edi

    dec     edx   
    cmp     edx, 0 
    jz      finish_addition
    jmp     process_larger_array 
    
finish_addition:
    mov     byte [edi], cl                
    
    mov     eax, dword [ebp - 4]          

    mov     esp, ebp
    pop     ebp
    ret

malloc_error:
    push    malloc_error_msg      
    call    puts
    push    1                     ; Exit with error code 1
    call    exit
    
; ===================== MAIN =====================
main:
    push    ebp                    ; Save old base pointer to stack
    mov     ebp, esp               ; Set new base pointer to current stack pointer (creates stack frame)
    
    mov     eax, [ebp + 2 * 4]     ; Get argc (1st param) from stack: ebp + 8 (2 * 4 bytes)
    mov     ebx, [ebp + 3 * 4]     ; Get argv (2nd param) from stack: ebp + 12 (3 * 4 bytes)
    
    cmp     eax, 1                 ; Compare argc with 1 (program name only, no arguments)
    jne     check_flags             ; If argc != 1, jump to check command line flags
    
    mov     eax, x_struct          ; Load address of first structure into eax
    push    eax                    ; Push address as parameter for print_struct
    call    print_struct           ; Call print_struct function
    
    mov     ebx, y_struct          ; Load address of second structure into ebx
    push    ebx                    ; Push address as parameter for print_struct
    call    print_struct           ; Call print_struct function
    
    jmp     add_and_print              ; Jump to addition routine (default behavior)
