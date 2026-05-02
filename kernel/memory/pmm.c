
// The idea is to implement a bitmap allocator for now

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <memory/include/pmm.h>


__attribute__((used, section(".limine_requests")))
static volatile struct limine_paging_mode_request liminePagingRequest = {
	.id = LIMINE_PAGING_MODE_REQUEST_ID,
	.revision = 6,

	.mode = LIMINE_PAGING_MODE_X86_64_4LVL,
	.max_mode = LIMINE_PAGING_MODE_X86_64_5LVL,
	.min_mode = LIMINE_PAGING_MODE_X86_64_MIN
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request limineMemmapRequest = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 6
};


static uint64_t totalMemory = 0;
static uint64_t pmmBitmap[4];
static size_t pmmBitmapSize;


static void bitmapSetContiguos(uint64_t* memAddress, size_t nPages) {
	uint64_t bitmapCoord = ((uint64_t)memAddress / (uint64_t)memoryPageSize);
	size_t row = bitmapCoord / 64;
	uint8_t column = bitmapCoord % 8;

	for (size_t i = 0; i < nPages; i++) {
		if (column >= 63) {
			column = 0;
			row++;
		}

		pmmBitmap[row] |= (1 << (column));
		column++;
	}
}

static void bitmapClearContiguos(uint64_t* memAddress, size_t nPages) {
	uint64_t bitmapCoord = ((uint64_t)memAddress / (uint64_t)memoryPageSize);
	size_t row = bitmapCoord / 64;
	uint8_t column = bitmapCoord % 8;

	for (size_t i = 0; i < nPages; i++) {
		if (column >= 63) {
			column = 0;
			row++;
		}

		pmmBitmap[row] &= ~(1 << (column));
		column++;
	}
}

static bool bitmapTestPage(uint64_t* memAddress) {
	uint64_t bitmapCoord = ((uint64_t)memAddress / (uint64_t)memoryPageSize);
	
	return (pmmBitmap[bitmapCoord / 64] & (1 << (bitmapCoord % 8)));
}


void pmmInit(void) {
	// So by now paging is already enabled by Limine
	if (liminePagingRequest.response != NULL) return;

	for (uint64_t i = 0; i < limineMemmapRequest.response->entry_count; i++) {
		if (limineMemmapRequest.response->entries[i]->type == LIMINE_MEMMAP_USABLE) {
			totalMemory += limineMemmapRequest.response->entries[i]->length;
		}
	}

	pmmBitmapSize = totalMemory / (memoryPageSize * 64);
	if (totalMemory % (memoryPageSize * 64) != 0) pmmBitmapSize++;
}