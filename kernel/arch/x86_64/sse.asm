
CPU X64
DEFAULT REL

SECTION .text

; * NOTE: SSE isn't used for now(see compiler flags) because I don't save XMM registers when an ISR pops up.
; * I still enable SSE because why not.


; See Intel manual volume 3A chapter 12.6
GLOBAL sseEnable
sseEnable:
	; We should expect at least SSE2
	mov eax, 1
	cpuid
	test edx, 1 << 25
	jz .noSse
	test edx, 1 << 26
	jz .noSse

	; SSE1 and SSE2(legacy SSE). See https://osdev.wiki/wiki/SSE
	mov rax, cr0
	and ax, 0xFFFB ; Clear CR0.EM(bit 2)
	or ax, 2 ; Set CR0.MP(bit 1)
	mov cr0, rax

	mov rax, cr4
	or ax, 3 << 9 ; Set CR4.OSFXSR and CR4.OSXMMEXCPT(bits 9 and 10)
	mov cr4, rax

	; * Note: it's probably a good idea to configure MXCSR

	; AVX
	; Let's check for XSAVE feature set avaiability
	; See Intel manual volume 1 chapter 13 and 14
	; test ecx, 0x8000000 ; bit 27
	; jz .noAvx

	; mov rax, cr4
	; or rax, 0x40000 ; Set CR4.OSXSAVE(bit 18)
	; mov cr4, rax

	; test ecx, 0x10000000 ; Check for AVX(bit 28)
	; jz .noAvx

	; xor ecx, ecx
	; xgetbv ; XCR0 is in edx:eax

	; push rax
	; and eax, 6
	; cmp eax, 6
	; je .avxOk

	; pop rax
	; or eax, 6 ; Set bit 1 and 2
	; xor ecx, ecx
	; xsetbv

	jmp .end

	.avxOk:
		pop rax

	.noSse:

	.noAvx:

	.end:
		ret