
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <arch/x86_64/include/gdt.h>
#include <arch/x86_64/include/idt.h>
#include <memory/include/pmm.h>
#include <drivers/include/serial.h>
#include <graphics/include/screen.h>
#include <graphics/include/print.h>

__attribute__((used, section(".limineRequestsStart"))) static volatile uint64_t limineRequestsStartMarker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limineRequests"))) static volatile uint64_t limineBaseRevision[] = LIMINE_BASE_REVISION(6);
__attribute__((used, section(".limineRequestsEnd"))) static volatile uint64_t limineRequestsEndMarker[] = LIMINE_REQUESTS_END_MARKER;

__attribute__((used, section(".limineRequests")))
static volatile struct limine_date_at_boot_request limineBootDateRequest = {
	.id = LIMINE_DATE_AT_BOOT_REQUEST_ID,
	.revision = 6
};


__attribute__((noreturn)) static void halt() {
	asm volatile("cli");
    for (;;) asm volatile("hlt");
}

// Yes, for now it's ugly
static void printDate(void) {
	int64_t unixTime = limineBootDateRequest.response->timestamp;
	uint32_t bootDays = (unixTime / 86400) + 719066;
	uint16_t bootYear = bootDays / 365;
	bootDays %= 365;
	uint8_t bootMonth = bootDays / 30;
	bootDays %= 30;

	unixTime /= 60;
	uint16_t bootMinutes = unixTime % 60;
	unixTime /= 60;
	uint16_t bootHour = unixTime % 24;
	unixTime /= 24;

	kPrintf("Boot time and date: %u:%u %u/%u/%u\n\n", bootHour, bootMinutes, bootDays, bootMonth, bootYear);
}

__attribute__((section(".entry"), noreturn)) void kernelMain(void) {
	asm volatile("cli");

	// If Limine revisions don't match we have quite a problem
	if (LIMINE_BASE_REVISION_SUPPORTED(limineBaseRevision) == false) halt();

	gdtInit();
	idtInit();
	
	asm volatile("sti");
	
	// Init graphics
	if (screenInit()) halt();

	// Display logo(yes I know it's not pretty to do it like this)
	kPrintf("\n%b ░█▀▀░█▀▀░█▀█░▀█▀░█░█░█▀▀░█▀▄░█▀█░█▀▀\n ░█▀▀░█▀▀░█▀█░░█░░█▀█░█▀▀░█▀▄░█░█░▀▀█\n ░▀░░░▀▀▀░▀░▀░░▀░░▀░▀░▀▀▀░▀░▀░▀▀▀░▀▀▀\n\n", def_text_info_colour);

	// Display boot date and time
	if (limineBootDateRequest.response != NULL) printDate();

	if (serialInit()) kPrintf("%bError%b > setup of serial ports driver failed!\n", def_text_error_colour, def_text_colour);
	if (pmmInit()) kPrintf("%bError%b > init of physical page frame allocator failed!\n", def_text_error_colour, def_text_colour);

	kPrintf("\n>> Nothing to do. Halting!");
    halt();
}