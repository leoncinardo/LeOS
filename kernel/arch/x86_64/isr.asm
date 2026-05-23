
CPU X64
DEFAULT REL
EXTERN isrExceptionHandler

SECTION .text

%macro setIsrInt 1
	GLOBAL isrStub%+%1
    isrStub%+%1:
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

	; * NOTE: For System V ABI only volatile registers should be saved, those are: rax, rcx, rdx, rsi, rdi, r8 to r11
	; To clean this up: on new hardware that supports APX two push/pop instructions can be replaced by push2p/pop2p.
	; I don't care enought to check for support so I'll leave it like this
	push rax
	push rcx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11

	mov rdi, rsp ; First argument for isrHandler
	call isrExceptionHandler

	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
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
		mov [rdi + i*8], rax

        %assign i i + 1
    %endrep

	ret


; Defines every CPU exception ISR
; https://osdev.wiki/wiki/Exceptions

setIsrInt 0 ; Division by 0
setIsrInt 1 ; Debug
setIsrInt 2 ; NMI
setIsrInt 3 ; Breakpoint
setIsrInt 4 ; Overflow
setIsrInt 5 ; Bound range exceeded
setIsrInt 6 ; Invalid opcode
setIsrInt 7 ; Device not avaiable
setIsrInt 8 ; Double fault
setIsrInt 9 ; Coprocessor segment overrun
setIsrInt 10 ; Invalid TSS
setIsrInt 11 ; Segment not present
setIsrInt 12 ; Stack-segment fault
setIsrInt 13 ; General protection fault
setIsrInt 14 ; Page fault
setIsrInt 15 ; Reserved
setIsrInt 16 ; x87 floating-point exception, unused
setIsrInt 17 ; Alignment check
setIsrInt 18 ; Machine check
setIsrInt 19 ; SIMD floating-point exception
setIsrInt 20 ; Virtualization exception
setIsrInt 21 ; Control protection exception
setIsrInt 22 ; |-----
setIsrInt 23 ; |
setIsrInt 24 ; |	All reserved
setIsrInt 25 ; |
setIsrInt 26 ; |
setIsrInt 27 ; |-----
setIsrInt 28 ; Hypervisor injection exception
setIsrInt 29 ; VMM communication exception
setIsrInt 30 ; Security exception
setIsrInt 31 ; Reserved


SECTION .bss

GLOBAL isrStubTable
isrStubTable: resq 32