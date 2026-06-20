
#include <include/string.h>

// For a faster to string algoritm see: https://medium.com/data-science/34-faster-integer-to-string-conversion-algorithm-c72453d25352


size_t strlen(const char *stringPtr) {
    const char *stringBase = stringPtr;

    for (; *stringBase; stringBase++);
    return stringPtr - stringBase;
}

char *intToString(int num, char *strBufferBase, const size_t bufferSize) {
    if (strBufferBase == NULL || bufferSize < 2) return NULL;

    uint8_t isNeg = 0;
    char *strPtr = strBufferBase + bufferSize - 1;
    *strPtr = '\0';

    if (!num) {
        if (bufferSize < 2) return NULL;
        *--strPtr = '0';

        return strPtr;
    }
        
    if (num < 0) {
        isNeg = 1;
        num = -num;
    }
    
    while (num && strPtr > strBufferBase) {
        *--strPtr = num % 10 + '0';
        num /= 10;
    }
    
    if (isNeg && strPtr > strBufferBase) *--strPtr = '-';

    return strPtr;
}

char *uintToString(uint32_t num, char *strBufferBase, const size_t bufferSize) {
    if (strBufferBase == NULL || bufferSize < 2) return NULL;

    char *strPtr = strBufferBase + bufferSize - 1;
    *strPtr = '\0';

    if (!num) {
        if (bufferSize < 2) return NULL;
        *--strPtr = '0';

        return strPtr;
    }
    
    while (num && strPtr > strBufferBase) {
        *--strPtr = num % 10 + '0';
        num /= 10;
    }

    return strPtr;
}

char *hexToString(uint64_t num, char *strBufferBase, const size_t bufferSize) {
    if (strBufferBase == NULL || bufferSize < 2) return NULL;
    
    static const char hexLookupChars[] = "0123456789ABCDEF";
    char *strPtr = strBufferBase + bufferSize - 1;
    *strPtr = '\0';

    if (!num) {
        if (bufferSize < 2) return NULL;
        *--strPtr = '0';

        return strPtr;
    }
    
    while (num && strPtr > strBufferBase) {
        *--strPtr = hexLookupChars[num % 16];
        num /= 16;
    }
    
    return strPtr;    
}
