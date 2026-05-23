
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

extern void sseEnable(void); 

__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t limineRequestsStartMarker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests"))) static volatile uint64_t limineBaseRevision[] = LIMINE_BASE_REVISION(6);
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t limineRequestsEndMarker[] = LIMINE_REQUESTS_END_MARKER;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_date_at_boot_request limineBootDateRequest = {
	.id = LIMINE_DATE_AT_BOOT_REQUEST_ID,
	.revision = 6
};


static void halt() {
    for (;;) asm volatile("hlt");
}


void __attribute__((section(".entry"))) kernelMain(void) {
	asm volatile("cli");

    // If revisions don't match we have quite a problem
    if (LIMINE_BASE_REVISION_SUPPORTED(limineBaseRevision) == false) halt();

	gdtInit();
	sseEnable();
	idtInit();
	
	asm volatile("sti");
	
	if (screenInit()) halt();
	screenPaintBackground(defScreenBackgroundColour);

	if (serialInit()) kPrintf("%bFailed:%b setup of serial ports driver\n", 0xED2139, 0xFFFFFF);
	if (pmmInit()) kPrintf("%bFailed:%b init of physical page frame allocator\n", 0xED2139, 0xFFFFFF);

	kPrintf("\n%b-- // FeatherOS\n\n", 0x67E544);
	kPrintf("sizeof gdtEntry_t: %u", sizeof(gdtEntry_t));

    halt();
}