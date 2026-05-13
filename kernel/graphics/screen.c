
#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <graphics/include/screen.h>
#include <graphics/include/print.h>
#include <graphics/include/font.h>


__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request limineFramebufferRequest = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 6
};


static struct limine_framebuffer* framebuffer = NULL;
static volatile uint32_t* framebufferPtr;
static uint64_t framebufferWidth;
static uint64_t framebufferHeight;
static uint64_t framebufferPitch;
static uint32_t framebufferBpp;


int kprintChar(const char c, uint32_t posX, uint32_t posY, uint32_t colour) {
	size_t charX, charY = 0;
	char* curChar = g_8x16_font + (c * fontCharHeight);

	for (charY; charY < fontCharHeight; charY++) {
		for (charX = 0; charX < fontCharWidth; charX++) {
			// If a pixel is present in both curChar and the mask display it
			if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(posX + charX) * framebufferBpp + (posY + charY)* framebufferPitch] = colour;
		}
	}

	return 0;
}


int kprint(const char* stringPtr, uint32_t posX, uint32_t posY, uint32_t colour) {
	uint32_t x = posX;
	uint32_t y = posY;
	char* curChar;
	size_t i, charX, charY = 0;

	for (i; stringPtr[i]; i++) {
		// Check if we can write the char
		if (x + fontCharWidth > framebufferWidth) {
			x = 0;
			y += fontCharHeight;
		}

		// 8 by 16 bits = 16 bytes per char so every byte describes a row
		curChar = g_8x16_font + (stringPtr[i] * fontCharHeight);

		for (charY = 0; charY < fontCharHeight; charY++) {
			for (charX = 0; charX < fontCharWidth; charX++) {
				// If a pixel is present in both curChar and the mask display it
				if (curChar[charY] & fontCharMask[charX]) framebufferPtr[(x + charX) * framebufferBpp + (y + charY)* framebufferPitch] = colour;
			}
		}

		x += fontCharWidth;
	}

	return 0;
}


void screenDrawRectangle(uint32_t posX, uint32_t posY, uint32_t width, uint32_t height, uint32_t colour) {
	if (posX + width > framebufferWidth || posY + height > framebufferHeight) return;

	uint32_t x, y;

	for (y = posY; y < posY + height; y++) {
		for (x = posX; x < posX + width; x++) {
			framebufferPtr[x * framebufferBpp + y * framebufferPitch] = colour;
		}
	}
}


void screenPaintBackground(uint32_t colour) {
	uint32_t x, y;

	for (y = 0; y < framebufferHeight; y++) {
		for (x = 0; x < framebufferWidth; x++) {
			framebufferPtr[x * framebufferBpp + y * framebufferPitch] = colour;
		}
	}
}


void screenInit(void) {
	if (limineFramebufferRequest.response == NULL || limineFramebufferRequest.response->framebuffer_count < 1) return;

	// We want a framebuffer with 32 bpp
	for (size_t i = 0; i < limineFramebufferRequest.response->framebuffer_count; i++) {
		if (limineFramebufferRequest.response->framebuffers[i]->bpp != 32) continue;

		framebuffer = limineFramebufferRequest.response->framebuffers[i];
		framebufferPtr = framebuffer->address;
		framebufferWidth = framebuffer->width;
		framebufferHeight = framebuffer->height;
		framebufferPitch = framebuffer->pitch / 4;
		framebufferBpp = framebuffer->bpp / 32;
		break;
	}

	if (framebuffer == NULL) return;

}