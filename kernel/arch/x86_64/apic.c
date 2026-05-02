
#include <arch/x86_64/include/apic.h>
#include <arch/x86_64/include/pic.h>


void lapicInit(void) {
	// First let's disable 8259 PIC
	picDisable();

	// TODO: continue
}