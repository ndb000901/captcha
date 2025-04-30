#include "captcha.h"


void generate_random_text(const char *charset, size_t charset_size, char *text, size_t length) {
    // srand((unsigned int)time(NULL));
    for (size_t i = 0; i < length; ++i) {
        text[i] = charset[rand() % charset_size];
    }
    text[length] = '\0';
}

static void draw_characters(captcha_context_ptr ptr, char *text, char* image) {
    int pen_x = 10;
    int pen_y = ptr->config.height / 2 + ptr->face->size->metrics.y_ppem / 2 - 10;

    for (int n = 0; n < strlen(text); n++) {
        if (FT_Load_Char(ptr->face, text[n], FT_LOAD_RENDER)) {
            fprintf(stderr, "Could not load character '%c'\n", text[n]);
            continue;
        }

        FT_Bitmap *bitmap = &(ptr->face)->glyph->bitmap;
        int x = pen_x + ptr->face->glyph->bitmap_left;
        int y = pen_y - ptr->face->glyph->bitmap_top;

        unsigned char r = 50 + rand() % 150;
        unsigned char g = 50 + rand() % 150;
        unsigned char b = 50 + rand() % 150;

        for (int row = 0; row < bitmap->rows; row++) {
            for (int col = 0; col < bitmap->width; col++) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < ptr->config.width && py >= 0 && py < ptr->config.height) {
                    unsigned char alpha = bitmap->buffer[row * bitmap->pitch + col];
                    int index = (py * ptr->config.width + px) * 3;
                    image[index + 0] = (alpha * r + (255 - alpha) * image[index + 0]) / 255;
                    image[index + 1] = (alpha * g + (255 - alpha) * image[index + 1]) / 255;
                    image[index + 2] = (alpha * b + (255 - alpha) * image[index + 2]) / 255;
                }
            }
        }

        pen_x += ptr->face->glyph->advance.x >> 6;
    }
}

static void draw_noise_points(captcha_context_ptr ptr, char* image) {
    int thickness = ptr->config.noise_points.thickness;
    int width = ptr->config.width;
    int height = ptr->config.height;
    for (int i = 0; i < ptr->config.noise_points.count; ++i) {
        int cx = rand() % width;
        int cy = rand() % height;
        unsigned char r = rand() % 256;
        unsigned char g = rand() % 256;
        unsigned char b = rand() % 256;

        for (int dy = -thickness / 2; dy <= thickness / 2; ++dy) {
            for (int dx = -thickness / 2; dx <= thickness / 2; ++dx) {
                int x = cx + dx;
                int y = cy + dy;
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    int index = (y * width + x) * 3;
                    image[index + 0] = r;
                    image[index + 1] = g;
                    image[index + 2] = b;
                }
            }
        }
    }
}

static void draw_noise_lines(captcha_context_ptr ptr, char* image) {
    int width = ptr->config.width;
    int height = ptr->config.height;
    int count = ptr->config.noise_lines.count;
    int thickness = ptr->config.noise_lines.thickness;
    for (int i = 0; i < count; ++i) {
        int x0 = rand() % width;
        int y0 = rand() % height;
        int x1 = rand() % width;
        int y1 = rand() % height;

        unsigned char r = rand() % 256;
        unsigned char g = rand() % 256;
        unsigned char b = rand() % 256;

        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;

        while (1) {
            for (int tx = -thickness / 2; tx <= thickness / 2; ++tx) {
                for (int ty = -thickness / 2; ty <= thickness / 2; ++ty) {
                    int nx = x0 + tx;
                    int ny = y0 + ty;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        int index = (ny * width + nx) * 3;
                        image[index + 0] = r;
                        image[index + 1] = g;
                        image[index + 2] = b;
                    }
                }
            }

            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
}


static int encode_png_to_file(const char *filename, unsigned char *png_data, int width, int height, int color_type, int bit_depth) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Could not open %s for writing\n", filename);
        return 1;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        fprintf(stderr, "Error: png_create_write_struct failed\n");
        return 1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        fclose(fp);
        fprintf(stderr, "Error: png_create_info_struct failed\n");
        return 1;
    }

    // 设置错误处理
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        fprintf(stderr, "Error during png creation\n");
        return 1;
    }

    png_init_io(png_ptr, fp);

    // 设置 PNG 文件的信息
    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    // 写入 PNG 文件头信息
    png_write_info(png_ptr, info_ptr);

    // 写入图像数据
    png_bytep row_pointers[height];
    int bytes_per_row = png_get_rowbytes(png_ptr, info_ptr);

    for (int y = 0; y < height; y++) {
        row_pointers[y] = png_data + y * bytes_per_row;
    }

    png_write_image(png_ptr, row_pointers);

    // 写入文件尾
    png_write_end(png_ptr, NULL);

    // 清理内存并关闭文件
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0;
}


// 自定义写入函数
static void write_png_data(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_memory_struct *mem_ptr = (png_memory_struct *)png_get_io_ptr(png_ptr);
    if (mem_ptr && data && length > 0) {
        unsigned char *new_data = (unsigned char *)realloc(mem_ptr->data, mem_ptr->size + length);
        if (new_data) {
            memcpy(new_data + mem_ptr->size, data, length);
        
            mem_ptr->data = new_data;
            mem_ptr->size += length;
        } else {
            // 处理内存分配失败的情况
            png_error(png_ptr, "Out of memory while writing PNG data");
        }
    }
}

// 自定义刷新函数 (可选，对于内存写入通常不需要)
static void flush_png_data(png_structp png_ptr) {
    // 对于内存写入，通常不需要刷新操作
}

static int encode_png_to_memory(unsigned char **png_bytes, size_t *png_size, unsigned char *raw_data, int width, int height, int color_type, int bit_depth) {
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Error: png_create_write_struct failed\n");
        return 1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        fprintf(stderr, "Error: png_create_info_struct failed\n");
        return 1;
    }

    png_memory_struct mem_data;
    mem_data.data = NULL;
    mem_data.size = 0;

    png_set_write_fn(png_ptr, &mem_data, write_png_data, flush_png_data);

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        if (mem_data.data) free(mem_data.data);
        fprintf(stderr, "Error during png creation\n");
        return 1;
    }

    // 设置 PNG 文件的信息
    png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_bytep row_pointers[height];
    int bytes_per_row = png_get_rowbytes(png_ptr, info_ptr);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = raw_data + y * bytes_per_row;
    }

    // 写入文件头信息
    png_write_info(png_ptr, info_ptr);

    // 写入图像数据
    png_write_image(png_ptr, row_pointers);

    // 写入文件尾
    png_write_end(png_ptr, NULL);

    // 将编码后的数据返回给调用者
    *png_bytes = mem_data.data;
    *png_size = mem_data.size;

    // 清理 png 结构体 (注意：我们不在这里释放 mem_data.data，因为调用者需要使用它)
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return 0;
}



int init_captcah_context(captcha_context_ptr ptr) {
    srand((unsigned int)time(NULL));

    if (FT_Init_FreeType(&(ptr->library))) {
        fprintf(stderr, "Could not init FreeType library\n");
        return 1;
    }

    if (FT_New_Face(ptr->library, ptr->config.font_path, 0, &(ptr->face))) {
        fprintf(stderr, "Could not open font\n");
        return 1;
    }
    FT_Set_Pixel_Sizes(ptr->face, 0, ptr->config.height / 1.3);
    return 0;
}

void destroy_captcha_context(captcha_context_ptr ptr) {
    FT_Done_Face(ptr->face);
    FT_Done_FreeType(ptr->library);
}

int gen_captcha(captcha_context_ptr ptr, const char *code, int count, unsigned char **data, int *size) {
    int width = ptr->config.width;
    int height = ptr->config.height;
    unsigned char *image = (unsigned char *)calloc(width * height * 3, sizeof(unsigned char));
    if (!image) {
        fprintf(stderr, "Could not allocate image buffer\n");
        return 1;
    }

    for (int i = 0; i < width * height * 3; ++i) {
        image[i] = 255;
    }

    char* text = malloc(count + 1);
    memcpy(text, code, count);
    text[count] = '\0';
    // char text[ptr->config.code_size + 1];
    // generate_text(text, ptr->config.code_size);
    // printf("Generated CAPTCHA: %s\n", text);

    draw_noise_points(ptr, image);
    draw_noise_lines(ptr, image);
    draw_characters(ptr, text, image);


    unsigned char *png_data = NULL;
    size_t png_data_size = 0;

    if (encode_png_to_memory(&png_data, &png_data_size, image, width, height, PNG_COLOR_TYPE_RGB, 8) == 0) {
        *data = png_data;
        *size = png_data_size;
    }
    else {
        *data = NULL;
        *size = 0;
        fprintf(stderr, "Failed to write image to memory\n");
        return 1;
    }
    free(image);
    return 0;
}

int gen_captcha_random(captcha_context_ptr ptr, unsigned char **data, int *size) {
    int width = ptr->config.width;
    int height = ptr->config.height;
    unsigned char *image = (unsigned char *)calloc(width * height * 3, sizeof(unsigned char));
    if (!image) {
        fprintf(stderr, "Could not allocate image buffer\n");
        return 1;
    }

    for (int i = 0; i < width * height * 3; ++i) {
        image[i] = 255;
    }


    char text[ptr->config.code_size + 1];
    generate_random_text(ptr->config.charset, ptr->config.charset_size, text, ptr->config.code_size);
    // printf("Generated CAPTCHA: %s\n", text);

    draw_noise_points(ptr, image);
    draw_noise_lines(ptr, image);
    draw_characters(ptr, text, image);


    unsigned char *png_data = NULL;
    size_t png_data_size = 0;

    if (encode_png_to_memory(&png_data, &png_data_size, image, width, height, PNG_COLOR_TYPE_RGB, 8) == 0) {
        *data = png_data;
        *size = png_data_size;
    }
    else {
        *data = NULL;
        *size = 0;
        fprintf(stderr, "Failed to write image to memory\n");
        return 1;
    }
    free(image);
    return 0;
}




