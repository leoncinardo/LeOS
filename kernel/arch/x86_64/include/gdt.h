
#pragma once

#include <stdint.h>

#define gdt_entries 6

#define kernel_code_seg 0x10
#define kernel_data_seg 0x20
#define user_code_seg 0x30
#define user_data_seg 0x40
#define kernel_tss_seg 0x50
#define tss_seg 0x60


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