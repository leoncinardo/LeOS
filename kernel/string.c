
#include <stdbool.h>
#include <include/string.h>

// For a faster to string algoritm see: https://medium.com/data-science/34-faster-integer-to-string-conversion-algorithm-c72453d25352


size_t strlen(const char *stringPtr) {
    const char *stringBase = stringPtr;

    for (; *stringBase; stringBase++);
    return stringPtr - stringBase;
}

char *intToString(int num, char *strBuffer, size_t bufferSize) {
    if (strBuffer == NULL || !bufferSize) return NULL;

    size_t i = 0;
    bool isNeg = false;

    if (!num) {
        if (bufferSize < 2) return NULL;
        strBuffer[0] = '0';
        strBuffer[1] = '\0';

        return strBuffer;
    }
        
    if (num < 0) {
        isNeg = true;
        num = -num;
    }
    
    while (num && i < bufferSize - 1) {
        strBuffer[i++] = num % 10 + '0';
        num /= 10;
    }
    
    if (isNeg && i < bufferSize - 1) strBuffer[i++] = '-';
    strBuffer[i] = '\0';

    // Now string is reversed so we need to reverse it
    char charToSwap;
    for (size_t k = 0; k < i - 1; k++, i--) {
        charToSwap = strBuffer[k];
        strBuffer[k] = strBuffer[i - 1];
        strBuffer[i - 1] = charToSwap;
    }

    return strBuffer;
}

char *uintToString(uint32_t num, char *strBuffer, size_t bufferSize) {
    if (strBuffer == NULL || !bufferSize) return NULL;

    size_t i = 0;

    if (!num) {
        if (bufferSize < 2) return NULL;
        strBuffer[0] = '0';
        strBuffer[1] = '\0';

        return strBuffer;
    }
    
    while (num && i < bufferSize - 1) {
        strBuffer[i++] = num % 10 + '0';
        num /= 10;
    }
    
    strBuffer[i] = '\0';

    // Now string is reversed so we need to reverse it
    char charToSwap;
    for (size_t k = 0; k < i - 1; k++, i--) {
        charToSwap = strBuffer[k];
        strBuffer[k] = strBuffer[i - 1];
        strBuffer[i - 1] = charToSwap;
    }

    return strBuffer;
}

char *hexToString(uint64_t num, char *strBuffer, size_t bufferSize) {
    if (strBuffer == NULL || bufferSize < 2) return NULL;
    
    static const char hexLookupChars[] = "0123456789ABCDEF";
    size_t i = 0;

    if (!num) {
        strBuffer[0] = '0';
        strBuffer[1] = '\0';
        return strBuffer;
    }
    
    while (num && i < bufferSize - 1) {
        strBuffer[i++] = hexLookupChars[num % 16];
        num /= 16;
    }
    
    strBuffer[i] = '\0';

    // Now string is reversed so we need to reverse it
    char charToSwap;
    for (size_t k = 0; k < i - 1; k++, i--) {
        charToSwap = strBuffer[k];
        strBuffer[k] = strBuffer[i - 1];
        strBuffer[i - 1] = charToSwap;
    }

    return strBuffer;    
}
