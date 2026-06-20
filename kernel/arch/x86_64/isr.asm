
CPU X64
DEFAULT REL
EXTERN isrExceptionHandler

; * NOTE: For System V ABI only volatile registers should be saved, those are: rax, rcx, rdx, rsi, rdi, r8 to r11

SECTION .text

%macro setIsrException 1
	GLOBAL isrStub%+%1
    isrStub%+%1:
		cli

		%ifn %1 == 8 || %1 == 10 || %1 == 11 || %1 == 12 || %1 == 13 || %1 == 14 || %1 == 17 || %1 == 21 || %1 == 29 || %1 == 30
			push 0 ; Fake error code
		%endif
		
		push %1 ; Interrupt index in isrStubTable

		jmp isrStubExceptionCommon

%endmacro

GLOBAL isrStubExceptionCommon
isrStubExceptionCommon:
	pushfq
	cld

	; To clean this up: on new hardware that supports APX two push/pop instructions can be replaced by push2p/pop2p.
	; For now don't care enought to check for support so I'll leave it like this
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	; Stack is already 16-byte aligned
	mov rdi, rsp ; First argument for isrHandler
	call isrExceptionHandler

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	popfq
	add rsp, 16 ; Remove errror code/fake-error code and int index from stack
	
    iretq


GLOBAL isrSetStubTable
isrSetStubTable:
	lea rdi, isrStubTable

	%assign i 0
    %rep 32
        lea rax, isrStub%+i
		mov [rdi + i * 8], rax

        %assign i i + 1
    %endrep

	ret


; Defines every CPU exception ISR
; https://osdev.wiki/wiki/Exceptions

setIsrException 0 ; Division by 0
setIsrException 1 ; Debug
setIsrException 2 ; NMI
setIsrException 3 ; Breakpoint
setIsrException 4 ; Overflow
setIsrException 5 ; Bound range exceeded
setIsrException 6 ; Invalid opcode
setIsrException 7 ; Device not avaiable
setIsrException 8 ; Double fault
setIsrException 9 ; Coprocessor segment overrun
setIsrException 10 ; Invalid TSS
setIsrException 11 ; Segment not present
setIsrException 12 ; Stack-segment fault
setIsrException 13 ; General protection fault
setIsrException 14 ; Page fault
setIsrException 15 ; Reserved
setIsrException 16 ; x87 floating-point exception, unused
setIsrException 17 ; Alignment check
setIsrException 18 ; Machine check
setIsrException 19 ; SIMD floating-point exception
setIsrException 20 ; Virtualization exception
setIsrException 21 ; Control protection exception
setIsrException 22 ; |-----
setIsrException 23 ; |
setIsrException 24 ; |	All reserved
setIsrException 25 ; |
setIsrException 26 ; |
setIsrException 27 ; |-----
setIsrException 28 ; Hypervisor injection exception
setIsrException 29 ; VMM communication exception
setIsrException 30 ; Security exception
setIsrException 31 ; Reserved


SECTION .bss

GLOBAL isrStubTable
isrStubTable: resq 32