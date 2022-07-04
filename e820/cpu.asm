[cpu 386]
[bits 16]

%define b byte
%define w word
%define d dword

%macro _enter 0
	push bp
	mov bp,sp
%endmacro

%macro _leave 0
	mov sp,bp
	pop bp
	retf
%endmacro


global _dosint
global _DetectCPU

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[section ._TEXT class=CODE align=16]

;;_TEXT segment public use16 para 'CODE'

;	uint32_t eax;
;	uint32_t ecx;
;	uint32_t edx;
;	uint32_t ebx;
;	uint32_t esi;
;	uint32_t edi;
;	uint32_t ebp;
;	uint16_t ds;
;	uint16_t es;
;	uint16_t fs;
;	uint16_t gs;
;	uint32_t flags;
align 2
_dosint:
	;; ax = int
	;; dx = struct

	_enter

	mov ax,w[bp + 6]
	mov b[cs: .l00],al
	mov d[cs: .l02],ebp

	pushad
	push ds
	push es
	push fs
	push gs

	lds bp,[bp + 8]
	mov d[cs: .l01],ebp

	mov eax,d[bp + 0]
	mov ecx,d[bp + 4]
	mov edx,d[bp + 8]
	mov ebx,d[bp + 12]
	mov esi,d[bp + 16]
	mov edi,d[bp + 20]
	mov ds,w[bp + 28]
	mov es,w[bp + 30]
	mov fs,w[bp + 32]
	mov gs,w[bp + 34]
	mov ebp,d[bp + 24]

	db 0xCD
.l00:
	db 0x00

	push ebp

	;; move bp
	db 0x66
	db 0xBD
.l01:
	dd 0

	;; eax
	mov d[bp + 0],eax
	pushfd
	;; flags
	pop eax
	mov d[bp + 36],eax
	mov w[bp + 34],gs
	mov w[bp + 32],fs
	mov w[bp + 30],es
	mov w[bp + 28],ds
	;; ebp
	pop eax
	mov d[bp + 24],eax
	mov d[bp + 20],edi
	mov d[bp + 16],esi
	mov d[bp + 12],ebx
	mov d[bp + 8],edx
	mov d[bp + 4],ecx

	db 0x66
	db 0xBD
.l02:
	dd 0

	pop gs
	pop fs
	pop es
	pop ds
	popad
	_leave


align 2
_DetectCPU:
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

