
#include <stdint.h>
#include <arch/x86_64/include/idt.h>


__attribute__((aligned(0x10))) static idtEntry_t idt[idtEntries];
extern void* isrStubTable[]; // This is from isr.asm
extern void isrSetStubTable(void); // And also this


void isrHandler(intStackFrame_t* stackFrame) {
	asm volatile("cli; hlt");

}


static void idtSetEntry(uint8_t i, uint64_t* isrAddr, uint8_t ist, uint8_t flags) {
	idt[i].isr0 = (uint16_t)((uint64_t)isrAddr & 0xFFFF);
	idt[i].kernelCs = kernelCodeSeg;
	idt[i].ist = ist;
	idt[i].flags = flags;
	idt[i].isr1 = ((uint64_t)isrAddr >> 16) & 0xFFFFFFFFFFFF;
	idt[i].reserved = 0;
}


void idtInit(void) {
	idtIdtr_t idtr;

	isrSetStubTable();

	for (uint8_t i = 0; i < 32; i++) {
		idtSetEntry(i, isrStubTable[i], 0, 0x8E);
	}

	idtr.size = (uint16_t)sizeof(idt) - 1;
	idtr.offset = (uint64_t)&idt;

	asm volatile("lidt %0" : : "m"(idtr));
}