; T DINESH RAM KUMAR
; 2014A3A70302P
; COMPILER SUBMISSION 

; rsi contains the src
; rax contains strlen
__strlen:
	xor  rax, rax
__strlen1:
	mov cl, [rsi]
	cmp cl, 0
	je __strlen2
	inc rsi
	inc rax
	jmp __strlen1
__strlen2:
	ret


; rdi contains the destination
; rsi contains the src
__strcpy:
	mov cl, [rsi] ; load a character
	mov [rdi], cl ; copy that character
	cmp cl, 0 ; check if end of string 
	je __strcpy1 ; if end of string return
	inc rsi ; increment source index
	inc rdi ; increment destination index
	jmp __strcpy ; go back
__strcpy1:
	ret

; rdi contains the destination
; rsi contains the first string
; rdx contains the second string
; output to rdi the concatination of strings in rsi and rdx
__strcat:
	xor rbx, rbx
__strcat1:	
	inc rbx ; keep track out count
	mov cl, [rsi] 
	mov [rdi], cl
	cmp cl, 0
	je __strcat2
	inc rsi
	inc rdi
	jmp __strcat1
__strcat2:
	cmp rbx, 24 ; maxstrlen = 24
	je __strcat3
	inc rbx
	mov cl, [rdx]
	mov [rdi], cl
	cmp cl, 0
	je __strcat3
	inc rdi
	inc rdx
	jmp __strcat2
__strcat3:
	xor rcx, rcx
	mov [rdi], cl	
	ret
