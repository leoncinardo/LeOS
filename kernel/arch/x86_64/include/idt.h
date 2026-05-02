
#pragma once

#include <arch/x86_64/include/gdt.h>

#define idtEntries 256 // Entries > 256 are ignored


typedef struct {
	uint64_t r11, r10, r9, r8;
	uint64_t rdi, rsi;
	uint64_t rdx, rcx, rax;
	uint64_t flags;

	uint64_t intIndex;
	uint64_t errorCode;

	// Stuff pushed by the CPU
	uint64_t rip;
	uint64_t cs; // Padded with 0s
	uint64_t cpuFlags;
	uint64_t rsp;
	uint64_t ss;

} __attribute__((packed)) intStackFrame_t;


typedef struct {
	uint16_t isr0;
	uint16_t kernelCs;
	uint8_t ist; // It's an offset into the IST in the TSS, if 0 the IST isn't used. Also there are 5 reserved bits
	uint8_t flags; // Gate type, 0, DPL(CPU privilege levels), and Present bit
	uint64_t isr1: 48;
	uint32_t reserved;

} __attribute__((packed)) idtEntry_t;

typedef struct {
	uint16_t size;
	uint64_t offset;

} __attribute__((packed)) idtIdtr_t;


void isrHandler();
void idtInit(void);