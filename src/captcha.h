#ifndef CAPTCHA_H_
#define CAPTCHA_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <png.h>

typedef struct {
    unsigned char *data;
    size_t size;
} png_memory_struct;


typedef struct {
    int count;
    int thickness;
} noise_points_config, *noise_points_config_ptr;

typedef struct {
    int count;
    int thickness;
} noise_lines_config, *noise_lines_config_ptr;

typedef struct {
    int width;
    int height;
    int code_size;
    const char* charset;
    int charset_size;
    char* font_path;
    noise_points_config noise_points;
    noise_lines_config noise_lines;

} captcha_config, *captcha_config_ptr;

typedef struct {
    FT_Library library;
    FT_Face face;
    captcha_config config;
} captcha_context, *captcha_context_ptr;

void generate_random_text(const char *charset, size_t charset_size, char *text, size_t length);
int init_captcah_context(captcha_context_ptr ptr);
void destroy_captcha_context(captcha_context_ptr ptr);
int gen_captcha(captcha_context_ptr ptr, const char *code, int count, unsigned char **data, int *size);
int gen_captcha_random(captcha_context_ptr ptr, unsigned char **data, int *size);
#endif