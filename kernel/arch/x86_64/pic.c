
#include <arch/x86_64/include/pic.h>
#include <arch/x86_64/include/ioPorts.h>


// Limine already masked all IRQs
void picDisable(void) {
	// https://helppc.netcore2k.net/hardware/8259
	outb(pic_master_command, 0x11);
	outb(pic_slave_command, 0x11);

	outb(pic_master_data, 0x20);
	outb(pic_slave_command, 0x28);

	outb(pic_master_command, 0x4);
	outb(pic_slave_command, 0x2);

	outb(pic_master_command, 0x1);
	outb(pic_slave_command, 0x1);

	// Masks every interrupt
	outb(pic_master_data, 0xFF);
	outb(pic_slave_data, 0xFF);

}