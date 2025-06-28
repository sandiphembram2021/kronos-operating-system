#include "kronos.h"

/* String utility functions */

/* Get string length */
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

/* Compare two strings */
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

/* Copy string */
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

/* Set memory */
void* memset(void* ptr, int value, size_t size) {
    unsigned char* p = (unsigned char*)ptr;
    while (size--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

/* Copy memory */
void* memcpy(void* dest, const void* src, size_t size) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (size--) {
        *d++ = *s++;
    }
    return dest;
}

/* String compare with length limit */
int strncmp(const char* str1, const char* str2, size_t n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

/* Find character in string */
char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == c) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}

/* Tokenize string */
char* strtok(char* str, const char* delim) {
    static char* last = NULL;
    
    if (str) {
        last = str;
    } else if (!last) {
        return NULL;
    }
    
    /* Skip leading delimiters */
    while (*last && strchr(delim, *last)) {
        last++;
    }
    
    if (!*last) {
        return NULL;
    }
    
    char* token_start = last;
    
    /* Find end of token */
    while (*last && !strchr(delim, *last)) {
        last++;
    }
    
    if (*last) {
        *last++ = '\0';
    }
    
    return token_start;
}
