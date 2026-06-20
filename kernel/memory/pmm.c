
/* The idea is to implement a hierarchical bitmap with 2 levels:
	level 0 -> 1 bit = 1 page
	level 1 -> 1 bit = 1 word in level 0

In level 0: if bit is clear/set -> corresponding page is unused/used
In level 1: if bit is clear/set -> corresponding word in level 0 is not all used/used completely

*/

#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <memory/include/pmm.h>
#include <include/string.h>
#include <graphics/include/print.h>

__attribute__((used, section(".limineRequests")))
static volatile struct limine_paging_mode_request liminePagingRequest = {
	.id = LIMINE_PAGING_MODE_REQUEST_ID,
	.revision = 6,

	.mode = LIMINE_PAGING_MODE_X86_64_4LVL,
	.max_mode = LIMINE_PAGING_MODE_X86_64_5LVL,
	.min_mode = LIMINE_PAGING_MODE_X86_64_MIN
};

__attribute__((used, section(".limineRequests")))
static volatile struct limine_memmap_request limineMemmapRequest = {
	.id = LIMINE_MEMMAP_REQUEST_ID,
	.revision = 6
};

__attribute__((used, section(".limineRequests")))
static volatile struct limine_hhdm_request limineHhdmRequest = {
	.id = LIMINE_HHDM_REQUEST_ID,
	.revision = 6
};

__attribute__((used, section(".limineRequests")))
static volatile struct limine_executable_address_request limineExecAddrRequest = {
	.id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
	.revision = 6
};


extern uint64_t kStartPtr[];
extern uint64_t kEndPtr[];
static struct limine_memmap_entry **memMapEntries;

struct {
	uint64_t totalMemory;
	size_t totalPages;
	size_t freePages;

	uint64_t *bitmap0;
	uint64_t *bitmap1;

	size_t size0; // In bytes
	size_t size1; // In bytes
	size_t entries0;
	size_t entries1;
	size_t bitmapsMemmapEntry;
	size_t lastFreeInd1;

} pmmCtx;


static inline void bitmap1Update(size_t bitmap0Ind) {
	size_t bitmap1Ind = bitmap0Ind / pmm_bits_per_word;
	uint64_t mask = 1ULL << (bitmap0Ind % pmm_bits_per_word);

	// If not all of the word is used it's 0
	uint64_t bitToChange = (uint64_t)(pmmCtx.bitmap0[bitmap0Ind] == UINT64_MAX) << (bitmap0Ind % pmm_bits_per_word);

	pmmCtx.bitmap1[bitmap1Ind] = (pmmCtx.bitmap1[bitmap1Ind] & ~mask) | bitToChange;
}

// *NOTE: This function assumes every page to set was free beforehand
static void bitmapSetRange(const void *startPtr, const size_t size) {
	if (!size) return;

	size_t startBit = (uintptr_t)startPtr / mem_page_size;
	size_t startInd = startBit / pmm_bits_per_word;
	size_t startOff = startBit % pmm_bits_per_word;
	size_t nBits = (size + (mem_page_size - 1)) / mem_page_size;
	pmmCtx.freePages -= nBits;

	size_t endBit = startBit + nBits;
	size_t endInd = (endBit - 1) / pmm_bits_per_word;
	size_t endOff = endBit % pmm_bits_per_word;

	if (startInd == endInd) {
		uint64_t mask = (startOff == 0 && nBits == pmm_bits_per_word) ? UINT64_MAX : ((1ULL << nBits) - 1) << startOff;

		pmmCtx.bitmap0[startInd] |= mask;
		bitmap1Update(startInd);
		return;
	}

	// Manually set the first 64 bits because we may not manipulate some bits
	pmmCtx.bitmap0[startInd] |= UINT64_MAX << startOff;
	bitmap1Update(startInd++);

	if (startInd < endInd) {
		for (size_t i = startInd; i < endInd; i++) {
			pmmCtx.bitmap0[i] = UINT64_MAX;
			pmmCtx.bitmap1[i / pmm_bits_per_word] |= 1ULL << (i % pmm_bits_per_word);
		}
	}

	pmmCtx.bitmap0[endInd] |= endOff ? (1ULL << endOff) - 1 : UINT64_MAX;
	bitmap1Update(endInd);
}

// Same logic as previous function, just do the opposite bit manipulation
// *NOTE: This function assumes every page to clear was set beforehand
static void bitmapClearRange(const void *startPtr, const size_t size) {
	if (!size) return;

	size_t startBit = (uintptr_t)startPtr / mem_page_size;
	size_t startInd = startBit / pmm_bits_per_word;
	size_t startOff = startBit % pmm_bits_per_word;

	size_t nBits = (size + (mem_page_size - 1)) / mem_page_size;
	pmmCtx.freePages += nBits;

	size_t endBit = startBit + nBits;
	size_t endInd = (endBit - 1) / pmm_bits_per_word;
	size_t endOff = endBit % pmm_bits_per_word;

	if (startInd == endInd) {
		uint64_t mask = (startOff == 0 && nBits == pmm_bits_per_word) ? UINT64_MAX : ((1ULL << nBits) - 1) << startOff;

		pmmCtx.bitmap0[startInd] &= ~mask;
		bitmap1Update(startInd);
		return;
	}

	pmmCtx.bitmap0[startInd] &= ~(UINT64_MAX << startOff);
	bitmap1Update(startInd++);

	if (startInd < endInd) {
		for (size_t i = startInd; i < endInd; i++) {
			pmmCtx.bitmap0[i] = 0ULL;
			pmmCtx.bitmap1[i / pmm_bits_per_word] &= ~(1ULL << (i % pmm_bits_per_word));
		}
	}
	
	pmmCtx.bitmap0[endInd] &= endOff ? ~((1ULL << endOff) - 1) : 0ULL;
	bitmap1Update(endInd);
}

void *pmmAllocPage(void) {
	if (!pmmCtx.freePages) return NULL;

	for (size_t i = pmmCtx.lastFreeInd1; i < pmmCtx.entries1; i++) {
		uint64_t word1 = pmmCtx.bitmap1[i];
		if (word1 == UINT64_MAX) continue;

		// See https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html
		// Find first clear bit in level 1
		int bit1 = __builtin_ctzll(~word1);
		size_t bitmap0Ind = i * pmm_bits_per_word + bit1;

		// Find first clear bit in level 0
		uint64_t word0 = pmmCtx.bitmap0[bitmap0Ind];
		int bit0 = __builtin_ctzll(~word0);

		// Set the actual bit in level 0
		word0 |= 1ULL << bit0;
		pmmCtx.bitmap0[bitmap0Ind] = word0;

		// Update level 1 bitmap
		if (word0 == UINT64_MAX) {
			word1 |= 1ULL << bit1;
			pmmCtx.bitmap1[i] = word1;
		}

		pmmCtx.lastFreeInd1 = i;
		pmmCtx.freePages--;

		return (void *)(((uintptr_t)bitmap0Ind * pmm_bits_per_word + (uintptr_t)bit0) * mem_page_size);
	}

	return NULL;
}

// This function was mostly made by Claude because I couldn't bother
void *pmmAllocRange(const size_t pages) {
	if (!pmmCtx.freePages || !pages || pmmCtx.freePages < pages) return NULL;
	if (pages == 1) return pmmAllocPage();

	size_t rangeBase = SIZE_MAX;
	size_t range = 0;

	// Let's find a region big enough to fit all the pages contiguosly
	for (size_t i = pmmCtx.lastFreeInd1; i < pmmCtx.entries1; i++) {
		uint64_t word1 = pmmCtx.bitmap1[i];
		
		if (word1 == UINT64_MAX) {
			rangeBase = SIZE_MAX;
			range = 0;
			
			continue;
		}
		
		for (uint64_t freeBits1 = ~word1; freeBits1; ) {
			const int bit1 = __builtin_ctzll(freeBits1);
			const size_t bitmap0Ind = i * pmm_bits_per_word + bit1;

			if (bitmap0Ind >= pmmCtx.entries0) break;
			uint64_t word0 = pmmCtx.bitmap0[bitmap0Ind];

			for (uint64_t freeBits = ~word0; freeBits; ) {
				const int rangeBaseLocal = __builtin_ctzll(freeBits);
				size_t startBit = bitmap0Ind * pmm_bits_per_word + rangeBaseLocal;
				size_t rangeLocal = 0;

				// Find out how many bits are there in the range
				for (size_t bit = rangeBaseLocal; bit < pmm_bits_per_word; ++bit) {
					if (!(freeBits & (1ULL << bit))) break;

					rangeLocal++;
				}

				if (rangeBase + range == startBit && range) {
					range += rangeLocal;

				} else {
					rangeBase = startBit;
					range = rangeLocal;
				}

				if (range >= pages) goto rangeFound;
				if (rangeBaseLocal + rangeLocal >= pmm_bits_per_word) break;

				// Clear up the bits of the range we just checked
				freeBits &= UINT64_MAX << (rangeBaseLocal + rangeLocal);
			}

			// If last bit is set -> no continuity
			if (word0 & (1ULL << 63)) {
				rangeBase = SIZE_MAX;
				range = 0;
			}

			freeBits1 &= freeBits1 - 1;
		}
	}

	return NULL;

rangeFound: ;
	void *address = (void *)(rangeBase * mem_page_size);
	bitmapSetRange(address, pages * mem_page_size);
	pmmCtx.lastFreeInd1 = rangeBase / (pmm_bits_per_word * pmm_bits_per_word);
	pmmCtx.freePages -= pages;

	return address;
}

void pmmFreePage(const void* addr) {
	size_t pageNum = (uintptr_t)addr / mem_page_size;

	pmmCtx.bitmap0[pageNum / pmm_bits_per_word] &= ~(1ULL << pageNum % pmm_bits_per_word);
	pmmCtx.freePages++;
}

void pmmFreeRange(const void *addr, const size_t pagesCount) {
	bitmapClearRange(addr, pagesCount * mem_page_size);
}

uint64_t pmmGetTotalMem(void) {
	return pmmCtx.totalMemory;
}

uint64_t pmmGetFreeMem(void) {
	return (uint64_t)pmmCtx.freePages * mem_page_size;
}

int pmmInit(void) {
	// So by now paging is already enabled by Limine. Let's do some checks just to be sure
	if (liminePagingRequest.response == NULL || limineHhdmRequest.response == NULL || limineExecAddrRequest.response == NULL) {
		kPrintf("%bError%b > bootloader responses are missing!\n", def_text_error_colour, def_text_colour);
		return 1;
	}

	pmmCtx.bitmap0 = NULL;
	pmmCtx.bitmap1 = NULL;
	pmmCtx.totalMemory = 0;
	pmmCtx.freePages = 0;

	uint64_t kSize = (uintptr_t)kEndPtr - (uintptr_t)kStartPtr;
	uint64_t entryCount = limineMemmapRequest.response->entry_count;
	uint64_t hhdmOffset = limineHhdmRequest.response->offset;
	memMapEntries = limineMemmapRequest.response->entries;

	// To calculate the size of level 0 bitmap: get the highest memory address so we can get the number of pages needed to cover the whole memory
	uint64_t highestMemAddr = 0;
	for (size_t i = 0; i < entryCount; i++) {
		struct limine_memmap_entry *entry = memMapEntries[i];
		if (entry->type == LIMINE_MEMMAP_RESERVED || entry->type == LIMINE_MEMMAP_BAD_MEMORY || entry->type == LIMINE_MEMMAP_FRAMEBUFFER) continue;

		uint64_t tempHighestAddr = entry->base + entry->length;
		if (tempHighestAddr > highestMemAddr) highestMemAddr = tempHighestAddr;

		pmmCtx.totalMemory += entry->length;
	}

	pmmCtx.totalPages = (highestMemAddr + (mem_page_size - 1)) / mem_page_size;
	pmmCtx.size0 = (pmmCtx.totalPages + (sizeof(uint64_t) - 1)) / sizeof(uint64_t);
	pmmCtx.entries0 = (pmmCtx.totalPages + (pmm_bits_per_word - 1)) / pmm_bits_per_word;

	pmmCtx.entries1 = (pmmCtx.entries0 + (pmm_bits_per_word - 1)) / pmm_bits_per_word;
	pmmCtx.size1 = (pmmCtx.entries1 + (sizeof(uint64_t) - 1)) / sizeof(uint64_t);

	// Find first entry that meets criteria to allocate the bitmaps
	for (size_t i = 0; i < entryCount; i++) {
		struct limine_memmap_entry *entry = memMapEntries[i];
		if (entry->type != LIMINE_MEMMAP_USABLE || entry->length < pmmCtx.size0 + pmmCtx.size1) continue;
		
		// Allocate level 0 bitmap and set every bit in it
		pmmCtx.bitmap0 = memset((void *)(entry->base + hhdmOffset), 0xFF, pmmCtx.size0);

		// Allocate level 1 bitmap and set every bit in it
		pmmCtx.bitmap1 = memset((void *)(entry->base + hhdmOffset + pmmCtx.size0), 0xFF, pmmCtx.size1);

		pmmCtx.bitmapsMemmapEntry = i;
		break;
	}

	// Check if the bitmap is actually physically present
	if (pmmCtx.bitmap0 == NULL || pmmCtx.bitmap1 == NULL) {
		kPrintf("%bError%b > could not allocate PMM bitmaps!\n", def_text_error_colour, def_text_colour);
		return 1;
	};

	// Every bit of level 0 bitmap is already set so we just clear the bits that correspond to usable pages(also updates level 1 bitmap)
	// *NOTE: it's probably better to do the opposite. For now this will do
	for (size_t i = 0; i < entryCount; i++) {
		struct limine_memmap_entry *entry = memMapEntries[i];
		if (entry->type != LIMINE_MEMMAP_USABLE) continue;

		bitmapClearRange((void *)entry->base, entry->length);
	}

	// If pmmCtx.totalPages isn't a multiple of pmm_bits_per_word there are some bits that correspond to pages that don't physically exist
	size_t validBits = pmmCtx.totalPages % pmm_bits_per_word;
	if (validBits) {
		pmmCtx.bitmap0[pmmCtx.entries0 - 1] |= ~0ULL << validBits;
		pmmCtx.bitmap1[pmmCtx.entries1 - 1] |= ~0ULL << validBits;
	}

	// Set bitmaps and kernel pages as used
	// Limine also guarantees the kernel will be physically continuous
	bitmapSetRange((void *)memMapEntries[pmmCtx.bitmapsMemmapEntry]->base, pmmCtx.size0 + pmmCtx.size1);
	bitmapSetRange((void *)((uintptr_t)kStartPtr - limineExecAddrRequest.response->virtual_base + limineExecAddrRequest.response->physical_base), kSize);

	kPrintf("%bPMM%b > %u MB of memory detected. %u MB of usable memory\n", def_text_info_colour, def_text_colour, pmmCtx.totalMemory / 1024000, pmmGetFreeMem() / 1024000);
	kPrintf("\t| bitmap lvl 0 physical addr = 0x%X\n", (void *)((uintptr_t)pmmCtx.bitmap0 - hhdmOffset));
	kPrintf("\t| bitmap lvl 1 physical addr = 0x%X\n", (void *)((uintptr_t)pmmCtx.bitmap1 - hhdmOffset));
	kPrintf("\t| size of bitmap lvl 0 = %u.%u KB\n", pmmCtx.size0 / 0x400, pmmCtx.size0 % 0x400);
	kPrintf("\t| size of bitmap lvl 1 = %u bytes\n", pmmCtx.size1);
	kPrintf("\t| kernel physical addr = 0x%X\n", (uintptr_t)kStartPtr - limineExecAddrRequest.response->virtual_base + limineExecAddrRequest.response->physical_base);
	kPrintf("\t| size of kernel = %u.%u KB\n", kSize / 0x400, kSize % 0x400);

	return 0;
}