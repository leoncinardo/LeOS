
#include <stdint.h>
#include <drivers/include/serial.h>
#include <arch/x86_64/include/ioPorts.h>


uint8_t serialRead(void) {
	while (!(inb(com1 + 5) & 1));

	return inb(com1);
}

void serialWrite(uint8_t c) {
	while (!(inb(com1 + 5) & 0x20));

	outb(com1, c);
}

int serialInit(void) {
	outb(com1 + 1, 0x0); // Disable interrupts

	// Set baud rate divisor
	outb(com1 + 3, 0x80); // Set DLAB bit in Line Control Register
	outb(com1, 0x3); // Divisor low
	outb(com1 + 1, 0x0); // Divisor high
	outb(com1 + 3, 0x3); // Configure Line Control Register(clear DLAB, 8 bits, 1 stop bit)

	outb(com1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outb(com1 + 4, 0xB); // Configure Modem Control Register(IRQs enabled, DTR and RST set)
	outb(com1 + 4, 0x1E); // Set loopback mode and test serial chip(DTR cleared)

	// Test serial chip
	outb(com1, 0xAE);
	if (inb(com1) != 0xAE) return 1;

	// If not faulty set chip to normal state
	outb(com1 + 4, 0xF);
	return 0;
}