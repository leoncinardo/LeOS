
#pragma once

#define gdtEntries 6

#define kernelCodeSeg 0x10
#define kernelDataSeg 0x20
#define userCodeSeg 0x30
#define userDataSeg 0x40
#define kernelTssSeg 0x50
#define userTssSeg 0x60


typedef struct {
	uint16_t limit0;
	uint32_t base0: 24;
	uint8_t access;
	uint8_t limit1: 4;
	uint8_t flags: 4;
	uint64_t base1: 40;
	uint32_t reserved;
	
} __attribute__((packed)) gdtEntry_t;


typedef struct {
	uint16_t size;
	uint64_t offset;

} __attribute__((packed)) gdtGdtr_t;


void gdtInit(void);