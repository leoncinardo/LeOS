
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <limine.h>
#include <graphics/include/screen.h>
#include <graphics/include/print.h>
#include <include/string.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request limineFramebufferRequest = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 6
};

static struct limine_framebuffer *framebuffer = NULL;
static volatile uint32_t *framebufferPtr;
static uint64_t framebufferWidth;
static uint64_t framebufferHeight;
static uint64_t framebufferPitch;
static uint32_t framebufferBpp;
extern const uint8_t font8x16[];

static uint32_t textPosX, textPosY = 0;
static uint32_t screenBackgroundColour, screenForegroundColour, screenAccentColour;

static void (*kPrintfSpecifiersFuncs[31])(va_list *restrict argsListPtr);


// -- kPrintf and subsequent functions -- 


int kPrintf(const char *restrict stringPtr, ...) {
	va_list argsList;
	va_start(argsList, stringPtr);

	char *curChar;
	char specifier;
	size_t charX, charY;

	for (size_t i = 0; stringPtr[i]; i++) {
		if (stringPtr[i] == '%') {
			specifier = stringPtr[++i];

			if (specifier == '%') goto kPrintfCharPrintLoop;
			if (specifier < 'A' || specifier > 'z') continue; // If not a letter ignore it

			// If a lowercase letter
			if (specifier >= 'a') {
				// Execute the function associated to a letter. Eg: "a" -> kPrintfSpecifiersFuncs[0]()
				kPrintfSpecifiersFuncs[specifier - 'a'](&argsList);

			} else if (specifier <= 'Z') {
				// I could do the same thing for uppercase letters but it would be a waste of memory for only 5 of them being used
				switch (specifier) {
					case 0x41: kPrintfSpecifiersFuncs[26](&argsList); break; // "A"
					case 0x45: kPrintfSpecifiersFuncs[27](&argsList); break; // "E"
					case 0x46: kPrintfSpecifiersFuncs[28](&argsList); break; // "F"
					case 0x47: kPrintfSpecifiersFuncs[29](&argsList); break; // "G"
					case 0x58: kPrintfSpecifiersFuncs[30](&argsList); break; // "X"

					default: continue;
				}
			}

		} else if (stringPtr[i] == ' ') {
			textPosX += fontCharWidth;
			continue;
		
		} else if (stringPtr[i] == '\n') {
			textPosX = 0;
			textPosY += fontCharHeight;
			continue;

		} else {
			kPrintfCharPrintLoop:
			if (textPosX + fontCharWidth > framebufferWidth) {
				textPosX = 0;
				textPosY += fontCharHeight;
			}

			curChar = font8x16 + (stringPtr[i] * fontCharHeight);

			for (charY = 0; charY < fontCharHeight; charY++) {
				uint32_t *charRow = (uint32_t*)(framebufferPtr + (textPosX * framebufferBpp + (textPosY + charY)* framebufferPitch));
				uint8_t charGlyph = curChar[charY];

				for (charX = 0; charX < fontCharWidth; charX++) {
					if (charGlyph & (0x80 >> charX)) charRow[charX] = screenForegroundColour;
				}
			}

			textPosX += fontCharWidth;
		}
	}

	va_end(argsList);
	screenForegroundColour = defScreenForegroundColour;

	return 0;
}

static inline void kPrintfDrawChar(uint8_t charToDraw) {
	size_t charX, charY;

	// Check if we can write the char
	if (textPosX + fontCharWidth > framebufferWidth) {
		textPosX = 0;
		textPosY += fontCharHeight;
	}

	// 8 by 16 bits = 16 bytes per char so every byte describes a row
	const char *curChar = font8x16 + (charToDraw * fontCharHeight);

	for (charY = 0; charY < fontCharHeight; charY++) {
		uint32_t *charRow = (uint32_t*)(framebufferPtr + (textPosX * framebufferBpp + (textPosY + charY)* framebufferPitch));
		uint8_t charGlyph = curChar[charY];

		// Check every pixel in a row
		for (charX = 0; charX < fontCharWidth; charX++) {
			// If pixel is in glyph and the mask(bit 7 shifted to the right by charX positions) then draw it
			if (charGlyph & (0x80 >> charX)) charRow[charX] = screenForegroundColour;
		}
	}

	textPosX += fontCharWidth;
}

static void kPrintfC(va_list *restrict argsListPtr) {
	int arg = va_arg(*argsListPtr, int);
	kPrintfDrawChar((uint8_t) arg);
}

static void kPrintfS(va_list *restrict argsListPtr) {
	char *arg = va_arg(*argsListPtr, char*);
	char *curChar;
	size_t i, charX, charY;

	for (i = 0; arg[i]; i++) {
		if (arg[i] == '\n') {
			textPosX = 0;
			textPosY += fontCharHeight;
		}
		
		kPrintfDrawChar(arg[i]);
	}
}

static void kPrintfD(va_list *restrict argsListPtr) {
	int arg = va_arg(*argsListPtr, int);
	char* stringBuffer = intToString(arg);

	for (size_t i = 0; stringBuffer[i]; i++) {
		kPrintfDrawChar(stringBuffer[i]);
	}
}

static void kPrintfU(va_list *restrict argsListPtr) {
	uint32_t arg = va_arg(*argsListPtr, uint32_t);
	char* stringBuffer = intToString(arg);

	for (size_t i = 0; stringBuffer[i]; i++) {
		kPrintfDrawChar(stringBuffer[i]);
	}
}

static void kPrintfChangeColour(va_list *restrict argsListPtr) {
	uint32_t arg = va_arg(*argsListPtr, uint32_t);
	screenForegroundColour = arg;
}

static void kPrintfPlaceholder(va_list *restrict argsListPtr) {
	return;
}

// Since the functions associated to the specifiers are defined above I can't assign them where i declared this array
static void (*kPrintfSpecifiersFuncs[31])(va_list *restrict argsListPtr) = {
	kPrintfPlaceholder, kPrintfChangeColour, kPrintfC, kPrintfD, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder,
	kPrintfD, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder,
	kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfS, kPrintfPlaceholder,
	kPrintfU, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder,
	kPrintfPlaceholder, // "A"
	kPrintfPlaceholder, // "E"
	kPrintfPlaceholder, // "F"
	kPrintfPlaceholder, // "G"
	kPrintfPlaceholder // "X"
};


// -- Other text output functions --


void kPrintChar(const char c, uint32_t posX, uint32_t posY) {
	size_t charX, charY;
	char* restrict curChar = font8x16 + (c * fontCharHeight);

	for (charY = 0; charY < fontCharHeight; charY++) {
		uint32_t *charRow = (uint32_t*)(framebufferPtr + (posX * framebufferBpp + (posY + charY)* framebufferPitch));
		uint8_t charGlyph = curChar[charY];

		for (charX = 0; charX < fontCharWidth; charX++) {
			if (charGlyph & (0x80 >> charX)) charRow[charX] = screenForegroundColour;
		}
	}

}

void kPrint(const char *restrict stringPtr, uint32_t posX, uint32_t posY) {
	uint32_t x = posX;
	uint32_t y = posY;
	char *curChar;
	size_t i, charX, charY;

	for (i = 0; stringPtr[i]; i++) {
		// Check if we can write the char
		if (x + fontCharWidth > framebufferWidth) {
			x = 0;
			y += fontCharHeight;
		}

		// 8 by 16 bits = 16 bytes per char so every byte describes a row
		curChar = font8x16 + (stringPtr[i] * fontCharHeight);

		for (charY = 0; charY < fontCharHeight; charY++) {
			uint32_t *charRow = (uint32_t*)(framebufferPtr + (x * framebufferBpp + (y + charY)* framebufferPitch));
			uint8_t charGlyph = curChar[charY];

			for (charX = 0; charX < fontCharWidth; charX++) {
				if (charGlyph & (0x80 >> charX)) charRow[charX] = screenForegroundColour;
			}
		}

		x += fontCharWidth;
	}

}


// -- Functions to manage framebuffers --


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

int screenInit(void) {
	if (limineFramebufferRequest.response == NULL || limineFramebufferRequest.response->framebuffer_count < 1) return 1;

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

	if (framebuffer == NULL) return 1;

	screenBackgroundColour = defScreenBackgroundColour;
	screenForegroundColour = defScreenForegroundColour;
	screenAccentColour = defScreenAccentColour;

	return 0;
}