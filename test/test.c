#include "captcha.h"
#include "base64.h"

int main(int argc, char **argv) {

    captcha_context ctx;
    ctx.config.width = 200;
    ctx.config.height = 50;
    ctx.config.code_size = 6;
    ctx.config.charset = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    ctx.config.charset_size = 32;
    ctx.config.font_path = "/home/hello/code/cpp/captcha/fonts/Tagesschrift-Regular.ttf";
    ctx.config.noise_points.count = 100;
    ctx.config.noise_points.thickness = 3;
    ctx.config.noise_lines.count = 5;
    ctx.config.noise_lines.thickness = 2;
    init_captcah_context(&ctx);
    unsigned char *data;
    int size = 0;

    // for (int i = 0; i < 1000; i++) {
    //     gen_captcha_random(&ctx, &data, &size);
    //     size_t base64_size = 0;
    //     char* base64 = base64_encode(data, size, &base64_size);
    //     if (i == 999) {
    //     printf("%s\n", base64);
    //     }
    //     // printf("%s\n", base64);
    //     free(data);
    //     free(base64);
    // }
    const char *code = "ABCDEF";
    gen_captcha(&ctx, code, 6, &data, &size);
    size_t base64_size = 0;
    char* base64 = base64_encode(data, size, &base64_size);
    printf("%s\n", base64);
    free(data);
    free(base64);
    destroy_captcha_context(&ctx);
    return 0;
}