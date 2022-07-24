;;
;; once.com - Stu George
;; returns errorlevel 1 when first run on that day
;; returns errorlevel 0 when run again that day
;;

[bits 16]
[cpu 8086]
[org 0x100]

%define w word
%define b byte

[section .text]
start:
	mov sp, end_stack

	;; resize memory
	mov bx, sp
	mov cl, 4
	shr bx, cl
	inc bx

	mov ah,0x4A
	int 0x21

check_dos_version:
	mov ah,0x30
	int 0x21

	cmp al,3
	jae check_cli

	mov dx,dos_err

print_exit:
	mov ah,9
	int 0x21
	xor ax,ax
	jmp short quit_prog

check_cli:
	;; command line
	mov si,0x81
	xor bx,bx
	mov dx,help_msg

.q0:
	lodsb
	cmp al,0x0D
	je check_date

	cmp al,0x20
	je .q0

	dec si
	lodsw

	;; return as being first run, but dont write file to disk
	cmp ax,'-f'
	je file_error
	cmp ax,'-r'
	je reset
	jmp short print_exit

reset:
	mov cx,-1
	mov dx,-1
	jmp short save_date

check_date:
	mov ah,0x2A
	int 0x21

	;; return flag
	xor ax, ax

date_cmp1:
	cmp cx, 0xFEFE
	jne save_date

date_cmp2:
	cmp dx, 0xFEFE
	je quit_prog

save_date:
	mov w[date_cmp1+2], cx
	mov w[date_cmp2+2], dx

	;; get our exec name from environment block
	mov ds,w[0x2c]
	xor si,si
.l3:
	lodsw
	dec si
	or ax,ax
	jnz .l3

	lodsw
	lodsb

	mov dx, si

	;; open file with name from psp
	mov ax,0x3D02
	int 0x21
	jc file_error

.l4:
	;; handle
	mov bx,ax

	;; pointer to start
	mov ax,0x4200
	xor cx,cx
	xor dx,dx
	int 0x21

	push cs
	pop ds
	push cs
	pop es

	;; write our code, ignore our stack
	mov ah,0x40
	mov cx, end_prog - 0x100
	mov dx,start
	int 0x21

	;; close
	mov ah,0x3E
	int 0x21

	;;
file_error:
	mov al,1

	;; quit with AL!
	;; 0 = we have run already today
	;; 1 = first run today
quit_prog:
	mov ah,0x4C
	int 0x21

[section .data align=1]
dos_err:  db 'DOS 3+ required',13,10
		  db '$'

help_msg: db 'once [-f]',13,10
		  db 13,10,'Stu George, v0.1',13,10
		  db '-f  force run',13,10
		  db '-r  reset for next run',13,10
		  db 13,10
		  db 'Returns errorlevel',13,10,
		  db '0 - already run today',13,10
		  db '1 - first time run today',13,10
		  db '$'

end_prog:

[section .bss align=16]
stack:
	resw 64
end_stack:
