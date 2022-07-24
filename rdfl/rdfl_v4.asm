;
;	  Title : Small RDF2 Stub Loader
;	 Author : Stuart 'Dark Fiber' George <sgeorge@mega-tokyo.com>
;	Version : 0.4
; Assembler : Nasm
;  Comments : Basic RDF Loader for .COM files.
;
; - Only loads single segment RDF files (!! .text only !!)
; - Relocates 16 + 32bit relocs.
; - i've done some size optimising on it (so may not be all that readable!)
; - 92bytes in size
;
; copy /b stub.com + {myfile}.rdf  output.com

%define b byte
%define w word
%define d dword

struc RDFRELOC
	rdfr_type:	resb	1
	rdfr_len:	resb	1
	rdfr_seg:	resb	1
	rdfr_offs:	resd	1
	rdfr_rlen:	resb	1	;; addr len 1/2/4 bytes
	rdfr_rseg:	resw	1	;; seg to reloc in
endstruc

struc RDFHEAD
	rdfh_magic: resb	6
	rdfh_dlen:	resd	1
	rdfh_hlen:	resd	1
endstruc

struc RDFSEG
	rdfs_type: resw 1
	rdfs_num: resw 1
	rdfs_res: resw 1
	rdfs_len: resd 1
endstruc

struc RDFOBJ
	rdfo_type:	resb	1
	rdfo_len:	resb	1
endstruc


[bits 16]
[org 0x100]

code:
	mov bp,0x100				;; reloc address
	mov si,end_code+4		;; code start

relocate_rdf:
	cmp w[si],'F2'
	jne quit

	mov bx,RDFHEAD_size - 4

	;; RDFSEG_size, RDFHEAD_size-4 both == 10

	mov di,si
	add di,w[si -4 + rdfh_hlen] ;; size of headers
	add di,bx ; RDFHEAD_size -4		;; end of header!
	mov cx,w[di + rdfs_len]
	add di,bx ; RDFSEG_size
	mov dx,w[si -4 + rdfh_hlen]
	add si,bx ; RDFHEAD_size -4

.k0:	cmp b[si + rdfo_type],0x01		;; reloc record
	jne .k1

.z2:	push	di
	mov eax,d[si + rdfr_offs]
	add di,ax

	;; test reloc size
	mov al,b[si + rdfr_rlen]
	sub al,2	;; 2 = 16bit
	jz	.z4
	sub al,2	;; 4 = 32bit
	jnz .k1

	;; 32bit reloc
	db 0x66		;; 0x66 == add d[di],dx -> add d[di],edx

	;; 16bit reloc
.z4:	add w[di],bp
	pop di

.k1:	mov al,b[si + rdfo_len]
	cbw
	;add	ax,RDFOBJ_size		;; == 2
	inc ax
	inc ax

	add si,ax
	sub dx,ax
	jnz .k0

.k99:
	;; end!
	xchg	di,ax

	mov di,0x100 - (k99 - k01)
	;lea	di,[bp-(k99-k01)]
	push	di
	push	cx
	mov si,k01
	mov cx,(k99 - k01)
k01:
	rep movsb
k99:
	pop cx
	xchg	si,ax
quit:
	ret

end_code:
