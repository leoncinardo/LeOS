
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <limine.h>
#include <graphics/include/screen.h>
#include <graphics/include/print.h>
#include <graphics/include/font.h>
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
static uint32_t framebufferBpp;
static uint64_t framebufferPitch;

extern const struct fontStruct font8x16;
static const uint8_t *currentFontBitmap;
static uint8_t currentFontWidth;
static uint8_t currentFontHeight;
static uint32_t textPosX, textPosY = 0;
static uint32_t backgroundColour, foregroundColour, accentColour;

static void (*kPrintfSpecifiersFuncs[31])(va_list *);


// -- kPrintf and subsequent functions -- 


void kPrintf(const char *restrict stringPtr, ...) {
	va_list argsList;
	va_start(argsList, stringPtr);

	for (size_t i = 0; (uint8_t)stringPtr[i]; i++) {
		if (stringPtr[i] == '%') {
			uint8_t specifier = stringPtr[++i];

			if (!specifier) break;
			if (specifier == '%') goto kPrintfCharPrint;
			if (specifier < 'A' || specifier > 'z') continue; // If not a letter ignore it

			// If a lowercase letter
			if (specifier >= 'a') {
				// Execute the function associated to a letter. Eg: "a" -> kPrintfSpecifiersFuncs[0]()
				kPrintfSpecifiersFuncs[specifier - 'a'](&argsList);

			} else if (specifier <= 'Z') {
				// I could do the same thing for uppercase letters but it would be a waste of memory for only 5 of them being used
				switch (specifier) {
					case 'A': kPrintfSpecifiersFuncs[26](&argsList); break;
					case 'E': kPrintfSpecifiersFuncs[27](&argsList); break;
					case 'F': kPrintfSpecifiersFuncs[28](&argsList); break;
					case 'G': kPrintfSpecifiersFuncs[29](&argsList); break;
					case 'X': kPrintfSpecifiersFuncs[30](&argsList); break;

					default: continue;
				}
			}

		} else if (stringPtr[i] == ' ') {
			textPosX += currentFontWidth;
			continue;
		
		} else if (stringPtr[i] == '\n') {
			textPosX = 0;
			textPosY += currentFontHeight;
			continue;

		} else if (stringPtr[i] == '\t') {
			textPosX += currentFontWidth * 4;
			if (textPosX >= framebufferWidth) {
				textPosX = 0;
				textPosY += currentFontHeight;
			}

			continue;

		} else {
			kPrintfCharPrint:
			if (textPosX + currentFontWidth > framebufferWidth) {
				textPosX = 0;
				textPosY += currentFontHeight;
			}

			const uint8_t *glyph = currentFontBitmap + ((uint8_t)stringPtr[i] * currentFontHeight);

			for (size_t charY = 0; charY < currentFontHeight; charY++) {
				uint32_t *glyphRow = (uint32_t *)(framebufferPtr + (textPosY + charY) * framebufferPitch + textPosX * framebufferBpp);
				uint8_t charGlyph = glyph[charY];

				for (size_t charX = 0; charX < currentFontWidth; charX++) {
					if (charGlyph & (0x80 >> charX)) glyphRow[charX] = foregroundColour;
				}
			}

			textPosX += currentFontWidth;
		}
	}

	va_end(argsList);
	foregroundColour = defTextColour;
}

static inline void kPrintfDrawChar(uint8_t charToDraw) {
	if (textPosX + currentFontWidth > framebufferWidth) {
		textPosX = 0;
		textPosY += currentFontHeight;
	}

	// 8 by 16 bits = 16 bytes per char so every byte describes a row
	const uint8_t *glyph = currentFontBitmap + (charToDraw * currentFontHeight);

	for (size_t charY = 0; charY < currentFontHeight; charY++) {
		uint32_t *glyphRow = (uint32_t *)(framebufferPtr + (textPosY + charY)* framebufferPitch + textPosX * framebufferBpp);
		uint8_t charGlyph = glyph[charY];

		// Check every pixel in a row
		for (size_t charX = 0; charX < currentFontWidth; charX++) {
			// If pixel is in glyph and the mask(bit 7 shifted to the right by charX positions) then draw it
			if (charGlyph & (0x80 >> charX)) glyphRow[charX] = foregroundColour;
		}
	}

	textPosX += currentFontWidth;
}

static void kPrintfC(va_list *argsListPtr) {
	int arg = va_arg(*argsListPtr, int);
	kPrintfDrawChar((uint8_t)arg);
}

static void kPrintfS(va_list *argsListPtr) {
	char *arg = va_arg(*argsListPtr, char*);

	for (size_t i = 0; arg[i]; i++) {
		if (arg[i] == '\n') {
			textPosX = 0;
			textPosY += currentFontHeight;
		}
		
		kPrintfDrawChar((uint8_t)arg[i]);
	}
}

static void kPrintfD(va_list *argsListPtr) {
	int arg = va_arg(*argsListPtr, int);
	char stringBuffer[13];
	intToString(arg, stringBuffer, 13);

	for (size_t i = 0; stringBuffer[i]; i++) {
		kPrintfDrawChar(stringBuffer[i]);
	}
}

static void kPrintfU(va_list *argsListPtr) {
	uint32_t arg = va_arg(*argsListPtr, uint32_t);
	char stringBuffer[13];
	uintToString(arg, stringBuffer, 13);

	for (size_t i = 0; stringBuffer[i]; i++) {
		kPrintfDrawChar(stringBuffer[i]);
	}
}

static void kPrintfX(va_list *argsListPtr) {
	uint64_t arg = va_arg(*argsListPtr, uint64_t);
	char stringBuffer[19];
	hexToString(arg, stringBuffer, 19);

	for (size_t i = 0; stringBuffer[i]; i++) {
		kPrintfDrawChar(stringBuffer[i]);
	}
}


static void kPrintfChangeColour(va_list *argsListPtr) {
	uint32_t arg = va_arg(*argsListPtr, uint32_t);
	foregroundColour = arg;
}

static void kPrintfPlaceholder(__attribute__((unused)) va_list *argsListPtr) {
	return;
}

// Since the functions associated to the specifiers are defined above I can't assign them where i declared this array
static void (*kPrintfSpecifiersFuncs[31])(va_list *argsListPtr) = {
	kPrintfPlaceholder, kPrintfChangeColour, kPrintfC, kPrintfD, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder,
	kPrintfD, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder,
	kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfS, kPrintfPlaceholder,
	kPrintfU, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder, kPrintfPlaceholder,
	kPrintfPlaceholder, // "A"
	kPrintfPlaceholder, // "E"
	kPrintfPlaceholder, // "F"
	kPrintfPlaceholder, // "G"
	kPrintfX // "X"
};


// -- Other text output functions --


void kPrintChar(const char c, uint32_t posX, uint32_t posY) {
	const uint8_t *glyph = currentFontBitmap + (c * currentFontHeight);

	for (size_t charY = 0; charY < currentFontHeight; charY++) {
		uint32_t *glyphRow = (uint32_t*)(framebufferPtr + (posX * framebufferBpp + (posY + charY)* framebufferPitch));
		uint8_t charGlyph = glyph[charY];

		for (size_t charX = 0; charX < currentFontWidth; charX++) {
			if (charGlyph & (0x80 >> charX)) glyphRow[charX] = foregroundColour;
		}
	}

}

void kPrint(const char *restrict stringPtr, uint32_t posX, uint32_t posY) {
	uint32_t x = posX;
	uint32_t y = posY;

	for (size_t i = 0; stringPtr[i]; i++) {
		// Check if we can write the char
		if (x + currentFontWidth > framebufferWidth) {
			x = 0;
			y += currentFontHeight;
		}

		// 8 by 16 bits = 16 bytes per char so every byte describes a row
		const uint8_t *glyph = currentFontBitmap + (stringPtr[i] * currentFontHeight);

		for (size_t charY = 0; charY < currentFontHeight; charY++) {
			uint32_t *glyphRow = (uint32_t*)(framebufferPtr + (x * framebufferBpp + (y + charY)* framebufferPitch));
			uint8_t charGlyph = glyph[charY];

			for (size_t charX = 0; charX < currentFontWidth; charX++) {
				if (charGlyph & (0x80 >> charX)) glyphRow[charX] = foregroundColour;
			}
		}

		x += currentFontWidth;
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
		framebufferBpp = framebuffer->bpp / 32;
		framebufferPitch = framebuffer->pitch / 4;
		break;
	}

	if (framebuffer == NULL) return 1;

	backgroundColour = defBackgroundColour;
	foregroundColour = defTextColour;
	accentColour = defAccentColour;

	currentFontBitmap = font8x16.font;
	currentFontWidth = font8x16.fontWidth;
	currentFontHeight = font8x16.fontHeight;

	return 0;
}