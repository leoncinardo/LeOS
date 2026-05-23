
#include <stdint.h>
#include <drivers/include/serial.h>
#include <arch/x86_64/include/ioPorts.h>


uint8_t serialRead(void) {
	while (!(inb(COM1 + 5) & 1));

	return inb(COM1);
}

void serialWrite(uint8_t c) {
	while (!(inb(COM1 + 5) & 0x20));

	outb(COM1, c);
}

int serialInit(void) {
	outb(COM1 + 1, 0x0); // Disable interrupts

	// Set baud rate divisor
	outb(COM1 + 3, 0x80); // Set DLAB bit in Line Control Register
	outb(COM1, 0x3); // Divisor low
	outb(COM1 + 1, 0x0); // Divisor high
	outb(COM1 + 3, 0x3); // Configure Line Control Register(clear DLAB, 8 bits, 1 stop bit)

	outb(COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outb(COM1 + 4, 0xB); // Configure Modem Control Register(IRQs enabled, DTR and RST set)
	outb(COM1 + 4, 0x1E); // Set loopback mode and test serial chip(DTR cleared)

	// Test serial chip
	outb(COM1, 0xAE);
	if (inb(COM1) != 0xAE) return 1;

	// If not faulty set chip to normal state
	outb(COM1 + 4, 0xF);
	return 0;
}