
#pragma once

#include <stdint.h>
#include <stddef.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *vl, const void *vr, size_t n);

size_t strlen(const char *stringPtr);
char *intToString(int num, char *strBuffer, size_t bufferSize);
char *uintToString(uint32_t num, char *strBuffer, size_t bufferSize);
char *hexToString(uint64_t num, char *strBuffer, size_t bufferSize);