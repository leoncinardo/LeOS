
CPU X64
DEFAULT REL

SECTION .text

GLOBAL gdtLoad
gdtLoad:
	lgdt [rdi]

	; Reload registers
	push 0x10 ; Kernel code seg
	lea rax, [rel .reloadCs]
	push rax
	retfq

	.reloadCs:
		mov ax, 0x20 ; Kernel data seg
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
		mov ss, ax

		ret
