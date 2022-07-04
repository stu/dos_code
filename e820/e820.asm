;; stupid e820 emulator with 63mb extended memory

[bits 16]
[org 0x100]
[cpu 386]

%define d dword
%define b byte
%define w word
%define q word

%macro proc16 1
align 2, db 0x90
%1:
%endmacro

%macro _enter 0
	push bp
	mov bp,sp
%endmacro

%macro _leave 0
	mov sp,bp
	pop bp
	ret
%endmacro

[section .text]
start:
old15:
	jmp init
db 0

cont: dd 0

new15:
	cmp eax,0xE820
	je e820_func
	jmp far [cs: old15]

return_err:
	mov ah,0x86
	stc
	retf 2

start_res_cmp:
e820_func:
	cmp edx,0x534D4150
	jnz return_err
	cmp ecx,20
	jb return_err

	clc

;EAX = 0000E820h
;EDX = 534D4150h ('SMAP')
;EBX = continuation value or 00000000h to start at beginning of map
;ECX = size of buffer for result, in bytes (should be >= 20 bytes)
;ES:DI -> buffer for result (see #00581)

	or ebx,ebx
	jnz .l2
	mov d[cs: cont],ebx

.l2:
	push esi
	push edi

	mov esi,map
	add esi,d[cs: cont]
	add d[cs: cont],20

	mov ecx,20
.l3:
	db 0x2E
	lodsb
	stosb
	loop .l3

	cmp esi,end_map
	jb .l4

	mov d[cs: cont],0

.l4:
	pop edi
	pop esi

	mov ebx,d[cs: cont]

	mov ecx,20
	mov eax,0x534D4150

	clc
	retf 2
end_res_cmp:

map:
	dq 0
map_base_ram_length: dq 0
	dd 1

map_base_ram_start_post:
	dq 0, 0x400
	dd 2

	dq 0x0E8000, 0x018000
	dd 2

	dq 0x0100000
map_ext_ram_length:
	dq 0 ;(1024*1024*62)
	dd 1

	dq 0x0FFFC0000, 0x040000
	dd 2
end_map:

align 16, db 0
end_tsr_part:

init:
	mov sp,stack_top
	mov ah,0x4A
	mov bx,sp
	shr bx,1
	shr bx,1
	shr bx,1
	shr bx,1
	int 0x21

	mov cx,sp
	mov di,bss_start
	sub cx,di
	xor ax,ax
	rep stosb

	;; print header
	mov ah,9
	mov dx,tex0
	int 0x21

	call DetectCPU
	cmp ax,3
	jae .q1

	mov ah,9
	mov dx,err_cpu
	int 0x21

	mov ah,0x4C
	int 0x21

.q1:
	int 0x12
	movzx eax,ax
	dec ax
	shl eax,10

	mov d[map_base_ram_length],eax
	mov d[map_base_ram_start_post],eax

	int 0x12
	movzx eax,ax
	dec ax
	mov ebx,640
	sub ebx,eax

	shl ebx,10
	mov d[map_base_ram_start_post + 8],ebx


	mov ax,0x8800
	int 0x15
	jc .q2
	;; allocated by ems?
	;or ax,ax
	;jz .q2

	movzx eax,ax
	shl eax,10
	mov d[map_ext_ram_length],eax
	jmp .q3
.q2:
	;; try cmos

	mov al,0x18
	out 0x70,al

	in al,0x71
	mov bh,al
	mov al,0x17
	out 0x70,al
	in al,0x71
	mov ah,bh

	;; ax = ck
	movzx eax,ax
	shl eax,10
	mov d[map_ext_ram_length],eax

.q3:

	;; get current ansi vector
	mov ax,0x3515
	int 0x21
	mov w[old15+2],es
	mov w[old15],bx

	;; check our signature
	mov si,start_res_cmp
	mov di,si
	mov cx,end_res_cmp - start_res_cmp
	repe cmpsb
	jne .l1

	;; we are in memory!

	;; reset our original vector address
	mov dx,w[es: old15+0]
	mov ds,w[es: old15+2]
	mov ax,0x2515
	int 0x21

	;; free memory
	mov ah,0x49
	int 0x21

	push cs
	pop ds

	;; we are unloaded!
	mov ah,9
	mov dx,tex2
	int 0x21

	;; quit
	mov ah,0x4c
	int 0x21

.l1:
	push ds
	pop es

	;; pretest
	mov eax,0xE820
	mov ecx,20
	mov edx,0x534D4150
	xor ebx,ebx
	movzx edi,sp
	sub edi,40
	int 0x15
	jc .q00

	mov dx,tex6
	mov ah,9
	int 0x21

	mov ah,0x4C
	int 0x21

.q00:

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
	mov si,ue820_name
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

	push ds
	mov ax,0x2515
	mov ds,bp
	mov dx,new15
	int 0x21
	pop ds

	mov ah,9
	mov dx,tex1
	int 0x21

	mov ah,9
	mov dx,tex4
	cmp bp,0xA000
	jb .l5
	int 0x21

.l5:
	mov ah,9
	mov dx,tex3
	int 0x21

	cmp d[map_ext_ram_length],0
	jnz .ll5

	mov ah,9
	mov dx,tex5
	int 0x21

.ll5:
	mov ax,es
	cmp ax,bp
	jz .l4

	;; only free if we are not ourselves
	mov ah,0x49
	int 0x21

	mov ah,0x4C
	int 0x21

.l4:
	;; we are in our own memory block, so TSR it
	mov ah,0x31
	mov dx,end_tsr_part
	shr dx,1
	shr dx,1
	shr dx,1
	shr dx,1
	int 0x21


proc16 alloc_block
	;; try and allocate upper memory
	mov ax,0x4800
	mov bx,end_tsr_part
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

DetectCPU:
	_enter

	pushf

	xor ax,ax
	push ax
	popf

	;; test high bits for 8088/8086
	pushf
	pop ax
	and ah,0xF0
	cmp ah,0xF0
	je .l99

	;; set the nt/iopl flags for 386+
	mov ah,0x70
	push ax
	popf

	pushf
	pop ax
	and ah,0x70
	jz .l99

	;; 386 at least_
	mov ax,3
	jmp short .l100

.l99:
	xor ax,ax

.l100:
	popf
	_leave

	;; MCB name
ue820_name: db 'uE820',0,0,0

tex0:	db 13,10
		db "Faux E820 Support Driver v1.01 - Dark Fiber",13,10
		db "$"

tex1:	db 'uE820 Installed in $'

tex3:	db ' memory.',13,10
		db "Run uE820 again to remove it from memory.",13,10
		db "$"

tex5:   db 13,10,"(If extended memory is listed in the map as 0 length",13,10
		db " you probably have EMS/XMS driver running)",13,10
		db "$"

tex4:	db 'high UMB$'

tex2:	db "uE820 removed from memory.",13,10
		db "$"

tex6:	db "Your BIOS already supports E820 functionality",13,10
		db "$"

err_cpu db 'You need an 80386 or better cpu',13,10
		db '$'

[section .bss]
bss_start:

alignb 16
stack_bottom:
resw 128
stack_top:
