
#include <stdbool.h>
#include <include/string.h>


// -- Mem functions --


void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = dest;
    const uint8_t *restrict psrc = src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}


// -- String manipulation --


static char numToStringBuffer[13];


size_t strlen(const char *restrict stringPtr) {
    size_t i = 0;

    while (stringPtr[i]) i++;

    return i;
}

char *intToString(int num) {
    size_t i = 0;
    bool isNeg = false;

    if (!num) numToStringBuffer[0] = '0';
    if (num < 0) {
        isNeg = true;
        num = -num;
    }
    
    while (num) {
        numToStringBuffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    if (isNeg) numToStringBuffer[i++] = '-';
    numToStringBuffer[i] = '\0';

    // Now string is reversed so we need to reverse it
    char charToSwap;
    for (size_t k = 0; k < i / 2; k++) {
        charToSwap = numToStringBuffer[k];
        numToStringBuffer[k] = numToStringBuffer[i - 1 - k];
        numToStringBuffer[i - 1 - k] = charToSwap;
    }

    return numToStringBuffer;
}

char *uintToString(uint32_t num) {
    size_t i = 0;

    if (!num) numToStringBuffer[0] = '0';
    
    while (num) {
        numToStringBuffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    numToStringBuffer[i] = '\0';

    // Now string is reversed so we need to reverse it
    char charToSwap;
    for (size_t k = 0; k < i / 2; k++) {
        charToSwap = numToStringBuffer[k];
        numToStringBuffer[k] = numToStringBuffer[i - 1 - k];
        numToStringBuffer[i - 1 - k] = charToSwap;
    }

    return numToStringBuffer;
}

int stringToInt(const char *restrict string) {
    int num = 0;
    bool isNeg = false;
    size_t i = 0;

    if (string[i] == '-') {
        isNeg = true;
        i++;

    } else if (string[i] == '+') i++;

    while (string[i] > '9' || string[i] < '0') {
        num = num * 10 + (string[i++] - '0');
    }

    if (isNeg) return num * -1;
    return num;
}