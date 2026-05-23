
#pragma once

#define defScreenBackgroundColour 0x1A1B25
#define defScreenForegroundColour 0xFFFFFF
#define defScreenAccentColour 0x67E544

#define fontCharWidth 8
#define fontCharHeight 16

void screenDrawRectangle(uint32_t posX, uint32_t posY, uint32_t width, uint32_t height, uint32_t colour);
void screenPaintBackground(uint32_t colour);
int screenInit(void);