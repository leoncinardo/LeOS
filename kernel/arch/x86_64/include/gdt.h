
#pragma once

#define gdtEntries 6

#define kernelCodeSeg 0x08
#define kernelDataSeg 0x10
#define userCodeSeg 0x18
#define userDataSeg 0x20
#define kernelTssSeg 0x28
#define userTssSeg 0x30


typedef struct {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base1;
	uint8_t access;
	uint8_t limit1: 4;
	uint8_t flags: 4;
	uint8_t base2;
	
} __attribute__((packed)) gdtEntry_t;


typedef struct {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base1;
	uint8_t access;
	uint8_t limit1: 4;
	uint8_t flags: 4;
	uint8_t base2;
	uint32_t base3;
	uint32_t reserved;

} __attribute__((packed)) gdtSysSeg_t;


typedef struct {
	uint16_t size;
	uint64_t offset;

} __attribute__((packed)) gdtGdtr_t;


void gdtInit(void);