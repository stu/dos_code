[bits 16]
[org 0x100]

%define w word
%define d dword

%macro proc16 1
[section .text]
align 2, db 0x90
%1:
%endmacro

[section .text]

start:
	jmp setup_isr
	db 0x90
alignb 2, db 0x90
;; we are at 0x104 so buffered enough for indos_buffer

indos_buffer equ start
old_int_11: dd 0

my_interrupt:
	;; test indos
	push ds
	push bx
	lds bx,[cs: indos_buffer]
	xor ax,ax
	xor ax,w[bx]
	pop bx
	pop ds
	jnz .finish_isr

	;; do work here
	nop
	nop

.finish_isr:
	jmp far [cs: old_int_11]

;; paragraph align for tsr memory block
alignb 0x10, db 0
end_tsr_part:

proc16 setup_isr
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov sp,end_stack
	mov bx,sp
	add bx,15
	shr bx,4
	mov ah,0x4A
	int 0x21

	;; hook int11 as example
	mov ax,0x3511
	int 0x21

	mov w[old_int_11+2],es
	mov w[old_int_11+0],bx

	;; test it is ourself or someone else
	mov di,bx
	mov si,my_interrupt
	mov cx,setup_isr
	sub cx,si
	repe cmpsb
	jnz .go_resident

	;; its us, so detach int 11
	mov dx,w[es: old_int_11+0]
	mov ds,w[es: old_int_11+2]
	mov ax,0x2511
	int 0x21

	;; free tsr memory
	mov ah,0x49
	int 0x21

	mov ax,cs
	mov ds,ax

	mov dx,msg_unhook
	mov ah,9
	int 0x21

	;; quit
	mov ah,0x4C
	int 0x21

.go_resident:
	call setup_tsr_memory_block
	;; tsr code block is in BP

	mov ds,bp

	;; setup insdos buffer
	mov ah,0x34
	int 0x21
	;; dos version dependant, 3.1+  lets hope we dont wrap segments...
	dec bx
	mov w[indos_buffer+2],es
	mov w[indos_buffer+0],bx

	mov ax,0x2511
	mov dx,my_interrupt
	int 0x21

	;; free environment
	mov ax,cs
	mov ds,ax

	mov es,w[0x2C]
	mov ah,0x49
	int 0x21

	mov dx,msg_hook
	mov ah,9
	int 0x21

	mov ax,cs
	cmp ax,bp
	jz .tsr_low_memory

	;; only free ourself if we are not ourselves (tsr in umb)
	mov ah,0x49
	int 0x21

	mov ah,0x4C
	int 0x21

.tsr_low_memory:
	;; go tsr
	mov dx,setup_isr		;; already aligned correctly for tsr
	shr dx,1
	shr dx,1
	shr dx,1
	shr dx,1
	mov ah,0x31
	int 0x21

proc16 setup_tsr_memory_block
	push ds
	pop es

	mov bp,ds

	mov ax,0x3000
	int 0x21
	cmp al,5
	jb .nd0

	push ax

	;; chain umb
	mov ax,0x5803
	mov bx,1
	int 0x21

	pop ax

.nd0:
	cmp al,3
	jb .alloc_low

	;; last fit allocation strategy
	mov ax,0x5801
	mov bx,2
	int 0x21

	call alloc_block
	jc .nd90

	mov es,ax

	;; if its not umb, dont use it!
	cmp ax,0xA000
	jae .alloc_is_good

	;; free
	mov ah,0x49
	int 0x21

	mov ax,cs
	mov es,ax

.alloc_low:
	;; reset mem strategy to low
	call reset_alloc_low
	call alloc_block
	jc .nd90

	;; is our block below our cs
	mov bx,cs
	cmp ax,bx
	jb .alloc_is_good

	mov es,ax
	mov ah,0x49
	int 0x21
	mov ax,cs
	mov es,ax
	jmp .nd90

.alloc_is_good:
	mov bp,ax

.nd90:
	;; fail!
	call reset_alloc_low

	;; free up old environment blocks
	mov es,w[cs: 0x2c]
	mov ah,0x49
	int 0x21


	;; set owner to itself so dos does not free it up
	mov ax,bp
	dec ax
	mov es,ax
	inc ax
	mov w[es: 1], ax

	;; own memory
	mov ax,cs
	mov ds,ax

	;; lets do a MCB fudge for naming
	mov si,mytsr_name
	mov di,8
	mov cx,4
	rep movsw

	;; copy down
	mov ax,cs
	mov ds,ax
	mov es,bp
	cmp ax,bp
	jz .skip_move

	xor si,si
	xor di,di
	mov cx,end_tsr_part
	shr cx,1
	rep movsw

.skip_move:
	mov ax,cs
	mov ds,ax
	mov es,ax
	ret


proc16 alloc_block
	;; try and allocate upper memory
	mov ax,0x4800
	mov bx,end_tsr_part ;- 0x100
	shr bx,1
	shr bx,1
	shr bx,1
	shr bx,1
	int 0x21
	ret

proc16 reset_alloc_low
	mov ax,0x3000
	int 0x21
	cmp al,5
	jb .nd99

	push ax
	;; unchain UMB's
	mov ax,0x5803
	xor bx,bx
	int 0x21
	pop ax

.nd99:
	;; reset allocation strategy
	cmp al,3
	jb .nd100
	mov ax,0x5801
	xor bx,bx
	int 0x21

.nd100:
	ret

[section .data]
	;; MCB name, must be 8 characters here
mytsr_name: db 'myTSR',0,0,0

msg_hook: db 'We have hooked int 0x11. Run again to remove',0x0d,0x0a,'$'
msg_unhook: db 'We have unhooked int 0x11 and removed TSR',0x0d,0x0a,'$'

[section .bss]
align 2
	resw 128
end_stack:
