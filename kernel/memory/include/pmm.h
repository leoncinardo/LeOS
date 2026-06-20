
#pragma once


#define mem_page_size 0x1000 // 4KB
#define pmm_bits_per_word 64


int pmmInit(void);
void *pmmAllocPage(void);
void *pmmAllocRange(const size_t pages);
void pmmFreePage(const void* addr);
void pmmFreeRange(const void *addr, const size_t pagesCount);
uint64_t pmmGetTotalMem(void);
uint64_t pmmGetFreeMem(void);