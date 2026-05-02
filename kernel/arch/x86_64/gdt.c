
#include <stdint.h>
#include <arch/x86_64/include/gdt.h>


// Note: it's probably needed to add other entries to the gdt for defining memory section with different privilegies, other than
// those I already put, eg: kernel data read-only
// ! IMPORTANT: A TSS is needed before enabling paging? because of, for example, task switching.


__attribute__((aligned(0x08))) static gdtEntry_t gdt[gdtEntries];
extern void gdtLoad(gdtGdtr_t* gdtr);


static void gdtSetEntry(uint8_t i, uint8_t access, uint8_t flags) {
	// In Long Mode base and limit values are ignored

	gdt[i].limit0 = 0;
	gdt[i].base0 = 0;
	gdt[i].base1 = 0;
	gdt[i].access = access;
	gdt[i].limit1 = 0;
	gdt[i].flags = flags;
	gdt[i].base2 = 0;
}


void gdtInit(void) {
	gdtSetEntry(0, 0, 0); // Null desc
	gdtSetEntry(1, 0x9A, 0xA); // Kernel code
	gdtSetEntry(2, 0x92, 0xC); // Kernel data
	gdtSetEntry(3, 0xFA, 0xA); // User code
	gdtSetEntry(4, 0xF2, 0xC); // User data
	
	// Tell the CPU where the table is and reload segment registers
	gdtGdtr_t gdtr;
	gdtr.size = (uint16_t)sizeof(gdt) - 1;
	gdtr.offset = (uint64_t)&gdt;
	
	gdtLoad(&gdtr);

}