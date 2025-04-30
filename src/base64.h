#ifndef BASE64_H
#define BASE64_H
#include <stdio.h>

char *base64_encode(const unsigned char *src, size_t len, size_t *out_len);
#endif