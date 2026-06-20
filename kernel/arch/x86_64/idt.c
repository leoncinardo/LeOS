
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/include/gdt.h>
#include <arch/x86_64/include/idt.h>
#include <arch/x86_64/include/pic.h>
#include <graphics/include/print.h>


__attribute__((aligned(0x10))) static idtEntry_t idt[idt_entries];
extern void *isrStubTable[];
extern void isrSetStubTable(void);

static const char *isrExceptionsNames[] = {
    "Division by 0",
    "Debug exception",
    "Non-maskable interrupt exception",
    "Breakpoint exception",
    "Overflow",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not avaiable",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "Reserved exception",
    "x87 floating-point exception",
    "Alignment check",
    "Machine check",
    "SIMD floating-point exception",
    "Virtualization exception",
    "Control protection exception",
    "Reserved exception",
    "Reserved exception",
    "Reserved exception",
    "Reserved exception",
    "Reserved exception",
    "Reserved exception",
    "Hypervisor injection exception",
    "VMM communication exception",
    "Security exception",
    "Reserved exception"
};


__attribute__((interrupt)) void isrExceptionHandler(intStackFrameException_t *stackFrame) {
	kPrintf("\n%bKERNEL PANIC!%b > %s\n", def_text_error_colour, def_text_colour, isrExceptionsNames[stackFrame->intIndex]);
    kPrintf("Interrupt index: %u  Error code: %u\n\n", stackFrame->intIndex, stackFrame->errorCode);
    kPrintf("RIP: 0x%X  CS: 0x%X  RFLAGS: 0x%X\n", stackFrame->rip, stackFrame->cs, stackFrame->cpuFlags);
    kPrintf("RBP: 0x%X  RSP: 0x%X  SS: 0x%X\n\n", stackFrame->rbp, stackFrame->rsp, stackFrame->ss);

    // General purpose registers
    kPrintf("RAX: 0x%X  RCX: 0x%X  RDX: 0x%X\n", stackFrame->rax, stackFrame->rcx, stackFrame->rdx);
    kPrintf("RSI: 0x%X  RDI: 0x%X  R8: 0x%X  R9: 0x%X\n", stackFrame->rsi, stackFrame->rdi, stackFrame->r8, stackFrame->r9);
    kPrintf("R10: 0x%X  R11: 0x%X  R12: 0x%X\n", stackFrame->r10, stackFrame->r11, stackFrame->r12);
    kPrintf("R13: 0x%X  R14: 0x%X  R15: 0x%X", stackFrame->r13, stackFrame->r14, stackFrame->r15);

    asm volatile("cli");
    for (;;) asm volatile("hlt");
}

static void idtSetEntry(uint8_t i, void *isrAddr, uint8_t ist, uint8_t flags) {
    idt[i].isr0 = ((uintptr_t)isrAddr & 0xFFFF);
    idt[i].kernelCs = kernel_code_seg;
    idt[i].ist = ist;
    idt[i].flags = flags;
    idt[i].isr1 = ((uintptr_t)isrAddr >> 16) & 0xFFFF;
    idt[i].isr2 = ((uintptr_t)isrAddr >> 32) & 0xFFFFFFFF;
    idt[i].reserved = 0;
}

void idtInit(void) {
    idtIdtr_t idtr;

    isrSetStubTable();

    // Set all exceptions
    for (size_t i = 0; i < 32; i++) {
        idtSetEntry(i, isrStubTable[i], 0, 0x8F);
    }

    idtr.size = sizeof(idt) - 1;
    idtr.offset = (uint64_t)&idt;

    asm volatile("lidt %0" : : "m"(idtr));
}