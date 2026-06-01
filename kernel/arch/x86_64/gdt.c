
#include <arch/x86_64/include/gdt.h>


// Note: it's probably needed to add other entries to the gdt for defining memory section with different privilegies, other than
// those I already put, eg: kernel data read-only


__attribute__((aligned(0x10))) static gdtEntry_t gdt[gdtEntries];
extern void gdtLoad(gdtGdtr_t *gdtr);


static void gdtSetEntry(uint8_t i, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
	if (i > gdtEntries - 1) return;

	gdt[i].access = access;
	gdt[i].flags = flags;

	// For non-system seg descriptors base and limit values are ignored
	if (base == 0 && limit == 0) {
		gdt[i].limit0 = 0;
		gdt[i].limit1 = 0;
		gdt[i].base0 = 0;
		gdt[i].base1 = 0;
		gdt[i].reserved = 0;

		return;
	}

	gdt[i].limit0 = (uint16_t)(limit & 0xFFFF);
	gdt[i].limit1 = (uint8_t)((limit >> 16) & 0xF);
	gdt[i].base0 = (uint32_t)(base & 0xFFFFFF);
	gdt[i].base1 = (base >> 24) & 0xFFFFFFFFFF;
	gdt[i].reserved = 0;
}


void gdtInit(void) {
	gdtSetEntry(0, 0, 0, 0, 0); // Null desc
	gdtSetEntry(1, 0, 0, 0x9A, 0xA); // Kernel code
	gdtSetEntry(2, 0, 0, 0x92, 0xC); // Kernel data
	gdtSetEntry(3, 0, 0, 0xFA, 0xA); // User code
	gdtSetEntry(4, 0, 0, 0xF2, 0xC); // User data
	// gdtSetEntry(5, , , 0x89, 0); // TSS
	
	// Tell the CPU where the table is and reload segment registers
	gdtGdtr_t gdtr;
	gdtr.size = (uint16_t)sizeof(gdt) - 1;
	gdtr.offset = (uint64_t)&gdt;
	
	gdtLoad(&gdtr);
}