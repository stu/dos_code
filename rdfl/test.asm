[bits 16]
[cpu 8086]

[section .text]
start:
	mov sp,end_stack

	mov si,hwstr
	call dump_string

	mov ah,0x4c
	int 0x21

dump_string:
	lodsb
	or al,al
	jz .l1

.l0:
	int 0x29
	cmp al,10
	jnz dump_string
	mov al,13
	jmp short .l0

.l1:
	ret

;[section .data]
hwstr: db 'Hello from my RDF file',10,0

[section .bss]
resw 256
end_stack:
