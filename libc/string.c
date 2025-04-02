#include "libc.h"

// Memory movement function (handles overlapping memory regions)
void* memmove(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    // Check if destination and source overlap
    if (d < s) {
        // Copy from beginning to end (forward)
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        // Copy from end to beginning (backward)
        for (size_t i = n; i > 0; i--) {
            d[i-1] = s[i-1];
        }
    }
    // If d == s, no copying is needed
    
    return dest;
}

// String length function
size_t strlen(const char* str) {
    const char* s = str;
    while (*s) {
        s++;
    }
    return s - str;
}

// String copy function
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

// String copy function with length limit
char* strncpy(char* dest, const char* src, size_t n) {
    char* original_dest = dest;
    size_t i;
    
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    
    return original_dest;
}

// String compare function
int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

// String compare function with length limit
int strncmp(const char* s1, const char* s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (int)(unsigned char)s1[i] - (int)(unsigned char)s2[i];
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

// String concatenation function
char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    while (*dest) {
        dest++;
    }
    strcpy(dest, src);
    return original_dest;
}

// String concatenation function with length limit
char* strncat(char* dest, const char* src, size_t n) {
    char* original_dest = dest;
    size_t dest_len = strlen(dest);
    size_t i;
    
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[dest_len + i] = src[i];
    }
    
    dest[dest_len + i] = '\0';
    return original_dest;
}

// Find character in string
char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char*)s;
        }
        s++;
    }
    return (c == '\0') ? (char*)s : NULL;
}

// Find last occurrence of character in string
char* strrchr(const char* s, int c) {
    const char* last = NULL;
    while (*s) {
        if (*s == (char)c) {
            last = s;
        }
        s++;
    }
    if (c == '\0') {
        return (char*)s;
    }
    return (char*)last;
}

// Find substring in string
char* strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') {
        return (char*)haystack;
    }
    
    size_t needle_len = strlen(needle);
    while (*haystack) {
        if (strncmp(haystack, needle, needle_len) == 0) {
            return (char*)haystack;
        }
        haystack++;
    }
    
    return NULL;
}

// Memory copy function
void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

// Memory set function
void* memset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;
    
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
    
    return s;
}

// Compare memory
int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
    
    return 0;  // Memory regions are identical
}

// String duplication
char* strdup(const char* str) {
    size_t len = strlen(str) + 1;  // Include null terminator
    char* new_str = (char*)malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

// String tokenization
char* strtok(char* str, const char* delim) {
    static char* last_str = NULL;
    
    // If str is NULL, use the last string
    if (!str) {
        str = last_str;
        if (!str) {
            return NULL;  // No string to tokenize
        }
    }
    
    // Skip leading delimiters
    str += strspn(str, delim);
    if (*str == '\0') {
        last_str = NULL;
        return NULL;
    }
    
    // Find the end of the token
    char* token_end = str + strcspn(str, delim);
    if (*token_end == '\0') {
        last_str = NULL;
    } else {
        *token_end = '\0';
        last_str = token_end + 1;
    }
    
    return str;
}

// Calculate span of characters in str that are in accept
size_t strspn(const char* str, const char* accept) {
    const char* s = str;
    
    while (*s) {
        const char* a = accept;
        int found = 0;
        while (*a) {
            if (*s == *a) {
                found = 1;
                break;
            }
            a++;
        }
        if (!found) {
            break;
        }
        s++;
    }
    
    return s - str;
}

// Calculate span of characters in str that are not in reject
size_t strcspn(const char* str, const char* reject) {
    const char* s = str;
    
    while (*s) {
        const char* r = reject;
        while (*r) {
            if (*s == *r) {
                return s - str;
            }
            r++;
        }
        s++;
    }
    
    return s - str;
}

// Convert an integer to a string with the specified base
void int_to_string(int value, char* str, int base) {
    // Handle 0 explicitly, otherwise empty string is printed
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    // Handle negative numbers only for base 10
    int negative = 0;
    if (value < 0 && base == 10) {
        negative = 1;
        value = -value;
    }

    // Process individual digits
    int i = 0;
    while (value != 0) {
        int remainder = value % base;
        str[i++] = (remainder < 10) ? remainder + '0' : remainder - 10 + 'a';
        value = value / base;
    }

    // Add negative sign if needed
    if (negative)
        str[i++] = '-';

    // Null terminate string
    str[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}
