
#pragma once

#define defBackgroundColour 0x1A1B25
#define defAccentColour 0x67E544
#define defTextColour 0xFFFFFF
#define defTextErrorColour 0xED2139
#define defTextInfoColour 0x00DFBF

void kPrintf(const char *restrict stringPtr, ...);
void kPrintChar(const char c, uint32_t posX, uint32_t posY);
void kPrint(const char *restrict stringPtr, uint32_t posX, uint32_t posY);