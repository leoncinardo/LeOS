
#include <arch/x86_64/include/pic.h>
#include <arch/x86_64/include/ioPorts.h>


void picDisable(void) {
	// https://helppc.netcore2k.net/hardware/8259
	outb(picMasterCommand, 0x11);
	outb(picSlaveCommand, 0x11);

	outb(picMasterData, 0x20);
	outb(picSlaveCommand, 0x28);

	outb(picMasterCommand, 0x4);
	outb(picSlaveCommand, 0x2);

	outb(picMasterCommand, 0x1);
	outb(picSlaveCommand, 0x1);

	// Masks every interrupt
	outb(picMasterData, 0xFF);
	outb(picSlaveData, 0xFF);

}