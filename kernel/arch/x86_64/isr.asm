
CPU X64
DEFAULT REL
EXTERN isrHandler

; * NOTE: I asked an ai model some stuff and it recommended to initialize isrStubTable at runtime,
; * so I did that(yes I wrote the code by myself).

SECTION .text

%macro setIsrInt 1
	static isrStub%+%1
    isrStub%+%1:
		; * NOTE: Looks like that for System V ABI only volatile registers should be saved, those are:
		; * rax, rcx, rdx, rsi, rdi, r8 to r11

		%ifn %1 == 8 || %1 == 10 || %1 == 11 || %1 == 12 || %1 == 13 || %1 == 14 || %1 == 17 || %1 == 21 || %1 == 29 || %1 == 30
			push 0 ; Fake error code
		%endif
		
		push %1 ; Interrupt index in isrStubTable

		jmp isrStubCommon

%endmacro


static isrStubCommon
isrStubCommon:
	pushfq
	cld

	; I just discovered the existance of push2p/pop2p
	push2p rax, rcx
	push2p rdx, rsi
	push2p rdi, r8
	push2p r9, r10
	push r11

	mov rdi, rsp ; First argument for isrHandler
	call isrHandler

	pop r11
	pop2p r10, r9
	pop2p r8, rdi
	pop2p rsi, rdx
	pop2p rcx, rax
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
isrStubTable:
    resq 32
