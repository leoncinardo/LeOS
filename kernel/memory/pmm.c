
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <memory/include/pmm.h>
#include <include/string.h>
#include <graphics/include/print.h>

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

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request limineHhdmRequest = {
	.id = LIMINE_HHDM_REQUEST_ID,
	.revision = 6
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_address_request limineExecAddrRequest = {
	.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
	.revision = 6
};


extern uint64_t kStartPtr[];
extern uint64_t kEndPtr[];
static uint64_t kSize;
static uint64_t kPhysicalAddr;
static uint64_t kVirtualAddr;
static uint64_t hhdmOffset;
static uint64_t totalMemory = 0;

static uint64_t *pmmBitmap = NULL;
static size_t pmmBitmapEntryIndex;
static size_t pmmBitmapSize; // In bytes
static size_t pmmPagesCount;
static size_t pmmFreePages = 0;
static struct limine_memmap_entry **memMapEntries;
static uint64_t memMapEntriesCount;


// TODO: Optimise the loops, it's O(size * 8) now...
// https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html
static void bitmapSetContiguous(const void *startPtr, size_t size) {
	size_t firstPage = ((uint64_t)startPtr / memoryPageSize);
	size_t bitmapRow = firstPage / pmmBitmapBitsPerEntry;
	size_t bitmapCol = firstPage % pmmBitmapBitsPerEntry;
	size_t nPages = (size + memoryPageSize - 1) / memoryPageSize;

	for (size_t i = 0; i < nPages; i++, bitmapCol++, pmmFreePages--) {
		if (bitmapCol > pmmBitmapBitsPerEntry - 1) {
			bitmapCol = 0;
			bitmapRow++;
		}

		pmmBitmap[bitmapRow] |= (uint64_t)1 << bitmapCol;
	}
}

static void bitmapClearContiguous(const void *startPtr, size_t size) {
	size_t firstPage = ((uint64_t)startPtr / memoryPageSize);
	size_t bitmapRow = firstPage / pmmBitmapBitsPerEntry;
	size_t bitmapCol = firstPage % pmmBitmapBitsPerEntry;
	size_t nPages = (size + memoryPageSize - 1) / memoryPageSize;

	for (size_t i = 0; i < nPages; i++, bitmapCol++, pmmFreePages++) {
		if (bitmapCol > pmmBitmapBitsPerEntry - 1) {
			bitmapCol = 0;
			bitmapRow++;
		}

		pmmBitmap[bitmapRow] &= ~((uint64_t)1 << bitmapCol);
	}
}

// void pmmAllocPages(const void *startPtr, size_t size) {

// }

uint64_t pmmGetFreeMem(void) {
	return (uint64_t)pmmFreePages * memoryPageSize;
}

uint64_t pmmGetTotalMem(void) {
	return totalMemory;
}

int pmmInit(void) {
	// So by now paging is already enabled by Limine
	if (liminePagingRequest.response == NULL) return 1;
	if (limineHhdmRequest.response == NULL) return 1;
	if (limineExecAddrRequest.response == NULL) return 1;

	memMapEntries = limineMemmapRequest.response->entries;
	memMapEntriesCount = limineMemmapRequest.response->entry_count;
	hhdmOffset = limineHhdmRequest.response->offset;
	kPhysicalAddr = limineExecAddrRequest.response->physical_base;
	kVirtualAddr = limineExecAddrRequest.response->virtual_base;
	kSize = (uint64_t)kEndPtr - (uint64_t)kStartPtr;

	// To calculate the size of the bitmap get the highest memory address so we can get the number of pages needed to cover the whole memory
	uint64_t highestMemAddr = 0;
	uint64_t memType, tempHighestAddr;
	for (size_t i = 0; i < memMapEntriesCount; i++) {
		memType = memMapEntries[i]->type; // Just to make it more readable
		if (memType == LIMINE_MEMMAP_RESERVED || memType == LIMINE_MEMMAP_BAD_MEMORY || memType == LIMINE_MEMMAP_FRAMEBUFFER) continue;

		tempHighestAddr = memMapEntries[i]->base + memMapEntries[i]->length;
		if (tempHighestAddr > highestMemAddr) highestMemAddr = tempHighestAddr;

		totalMemory += memMapEntries[i]->length;
	}

	pmmPagesCount = (highestMemAddr + memoryPageSize - 1) / memoryPageSize;
	pmmBitmapSize = (pmmPagesCount + 7) / 8;

	// Find first entry that meets criteria for allocating the bitmap
	for (size_t i = 0; i < memMapEntriesCount; i++) {
		if (memMapEntries[i]->type != LIMINE_MEMMAP_USABLE || memMapEntries[i]->length < pmmBitmapSize) continue;
		
		// Allocate the bitmap and set every bit in it
		pmmBitmap = memset((void *)(memMapEntries[i]->base + hhdmOffset), 0xFF, pmmBitmapSize);
		pmmBitmapEntryIndex = i;
		break;
	}

	if (pmmBitmap == NULL) return 1;

	// Every bit in the bitmap is already set so we just clear the bits that correspond to a usable page
	for (size_t i = 0; i < memMapEntriesCount; i++) {
		if (memMapEntries[i]->type != LIMINE_MEMMAP_USABLE) continue;

		bitmapClearContiguous((void *)memMapEntries[i]->base, memMapEntries[i]->length);
	}

	// Set bitmap and kernel pages as used
	// Limine also guarantees the kernel will be physically continuous
	bitmapSetContiguous((void *)((uint64_t)pmmBitmap - hhdmOffset), pmmBitmapSize);
	bitmapSetContiguous((void *)((uint64_t)kStartPtr - kVirtualAddr + kPhysicalAddr), kSize);

	kPrintf("%bPMM%b > %u MB of memory detected, %u MB of usable memory\n", defTextInfoColour, defTextColour, pmmGetTotalMem() / 1024000, pmmGetFreeMem() / 1024000);
	kPrintf("\t| %u memmap entries\n", memMapEntriesCount);
	kPrintf("\t| size of bitmap = %u.%u KB\n", pmmBitmapSize / 1024, pmmBitmapSize % 1024);
	kPrintf("\t| kernel physical address = 0x%X\n", (uint64_t)kStartPtr - kVirtualAddr + kPhysicalAddr);
	kPrintf("\t| size of kernel = %u.%u KB\n", kSize / memoryPageSize, kSize % memoryPageSize);

	return 0;
}