
#pragma once


#define memoryPageSize 0x1000 // 4KB
#define pmmBitmapBitsPerEntry 64


int pmmInit(void);
uint64_t pmmGetFreeMem(void);
uint64_t pmmGetTotalMem(void);