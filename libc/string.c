/*
 * string.c
 */
#include <string.h>
#include <types.h>

int strcmp(const char *str1, const char *str2) {
    int res = 0;
    while (!(res = *(unsigned char *)str1 - *(unsigned char *)str2) && *str2) ++str1, ++str2;

    if (res < 0)
        res = -1;
    if (res > 0)
        res = 1;

    return res;
}

int strncmp(const char *str1, const char *str2, size_t num) {
    int res = 0;
    while (num-- && !(res = *(unsigned char *)str1 - *(unsigned char *)str2) && *str2) ++str1, ++str2;
    if (res < 0)
        res = -1;
    if (res > 0)
        res = 1;

    return res;
}

char *strcpy(char *dst, const char *src) {
    char *cp = dst;
    while ((*(cp++) = *(src++)))
        ;
    return (dst);
}

char *strncpy(char *dst, const char *src, size_t num) {
    char *cp = dst;
    while (num-- && (*(cp++) = *(src++)))
        ;
    return dst;
}

char *strcat(char *dst, const char *src) {
    char *cp = dst;
    while (*cp) cp++;  // Find the end of dst
    while ((*(cp++) = *(src++)))
        ;  // Copy src to end of dst
    return dst;
}

char *strncat(char *dst, const char *src, size_t num) {
    char *cp = dst;
    while (*cp) cp++;  // Find the end of dst
    while (num-- && (*(cp++) = *(src++)))
        ;
    return dst;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len++])
        ;
    return len - 1;
}

size_t strnlen(const char *str, size_t num) {
    size_t len = 0;
    while (str[len++] && (len <= num))
        ;
    return len - 1;
}

char *strchr(const char *str, int ch) {
    while (*str && *str != (char)ch) str++;
    if (*str == (char)ch)
        return ((char *)str);
    return NULL;
}

void *memset(void *dst, char val, size_t count) {
    unsigned char *temp = (unsigned char *)dst;
    for (; count != 0; count--, temp[count] = val)
        ;
    return dst;
}

void *memcpy(void *dst, const void *src, size_t count) {
    const char *sp = (const char *)src;
    char *dp = (char *)dst;
    for (; count != 0; count--) *dp++ = *sp++;
    return dst;
}

int memcmp(const void *s1, const void *s2, size_t count) {
    int res = 0;
    uint8_t *ptr1 = (uint8_t *)s1;
    uint8_t *ptr2 = (uint8_t *)s2;

    while (count-- && !(res = *(unsigned char *)ptr1 - *(unsigned char *)ptr2)) ++ptr1, ++ptr2;
    if (res < 0)
        res = -1;
    if (res > 0)
        res = 1;

    return res;
}

size_t lfind(const char *str, const char accept) {
    size_t i = 0;
    while (str[i] != accept) {
        i++;
    }
    return (size_t)(str) + i;
}

size_t rfind(const char *str, const char accept) {
    size_t i = strlen(str) - 1;
    while (str[i] != accept) {
        if (i == 0)
            return UINT32_MAX;
        i--;
    }
    return (size_t)(str) + i;
}

char *strpbrk(const char *str, const char *accept) {
    while (*str != '\0') {
        const char *acc = accept;
        while (*acc != '\0') {
            if (*acc++ == *str) {
                return (char *)str;
            }
        }
        ++str;
    }
    return NULL;
}

size_t strspn(const char *str, const char *accept) {
    const char *ptr;
    const char *acc;
    size_t size = 0;
    for (ptr = str; *ptr != '\0'; ++ptr) {
        for (acc = accept; *acc != '\0'; ++acc) {
            if (*ptr == *acc) {
                break;
            }
        }
        if (*acc == '\0') {
            return size;
        } else {
            ++size;
        }
    }
    return size;
}

char *strtok_r(char *str, const char *delim, char **saveptr) {
    char *token;
    if (str == NULL) {
        str = *saveptr;
    }
    str += strspn(str, delim);
    if (*str == '\0') {
        *saveptr = str;
        return NULL;
    }
    token = str;
    str = strpbrk(token, delim);
    if (str == NULL) {
        *saveptr = (char *)lfind(token, '\0');
    } else {
        *str = '\0';
        *saveptr = str + 1;
    }
    return token;
}
