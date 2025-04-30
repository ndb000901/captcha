#include "base64.h"
// Base64 编码
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64_encode(const unsigned char *src, size_t len, size_t *out_len) {
    size_t olen = 4 * ((len + 2) / 3);
    char *out = (char *)malloc(olen + 1);
    if (!out) return NULL;

    char *p = out;
    for (size_t i = 0; i < len; i += 3) {
        int val = 0;
        int n = 0;

        for (int j = 0; j < 3; j++) {
            val <<= 8;
            if (i + j < len) {
                val |= src[i + j];
                n++;
            }
        }

        for (int j = 0; j < 4; j++) {
            if (j <= (n + 0)) {
                *p++ = base64_table[(val >> (18 - 6 * j)) & 0x3F];
            } else {
                *p++ = '=';
            }
        }
    }

    *p = '\0';
    if (out_len) *out_len = p - out;
    return out;
}
