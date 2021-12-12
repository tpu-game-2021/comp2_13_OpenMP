#pragma warning(disable: 4996)
#include <iostream>// 時間計測用：気にしないこと
#include <chrono>// 時間計測用：気にしないこと

// 画像処理ライブラリの導入
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

using namespace std::chrono;// 時間計測用：気にしないこと


// モノクロ化
bool monochrome(const char* filename)
{
    int width, height; //画像サイズの格納先
    int bpp; //一画素のバイト数
    unsigned char* pixels = stbi_load(filename, &width, &height, &bpp, 0);

    if (pixels == NULL) return false;// ファイルが読めなかった

    auto start = system_clock::now();// 時間計測用：気にしないこと

    // ■ OpenMPを使って並列化してください。
 #pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char* r = &pixels[(y * width + x) * bpp + 0];
            unsigned char* g = &pixels[(y * width + x) * bpp + 1];
            unsigned char* b = &pixels[(y * width + x) * bpp + 2];

            int lum = ((int)*r + (int)*g + (int)*b) / 3;
            *r = lum;
            *g = lum;
            *b = lum;
        }
    }

    // 時間計測用：気にしないこと
    auto end = system_clock::now();
    std::cout << duration_cast<microseconds>(end - start).count() << " micro sec. \n";

    // ファイルの保存
    stbi_write_png("monochrome.png", static_cast<int>(width), static_cast<int>(height), bpp, pixels, 0);
    // メモリの破棄
    stbi_image_free(pixels);

    return true;
}

// ぼかす
bool blur(const char *filename, int num)
{
    int width, height; //画像サイズの格納先
    int bpp; //一画素のバイト数
    unsigned char* pixels = stbi_load(filename, &width, &height, &bpp, 0);

    if (pixels == NULL) return false;// ファイルが読めなかった

    auto start = system_clock::now();// 時間計測用：気にしないこと

    // ■ OpenMPを使って並列化してください。
    // 依存性があり、並列化すると処理の順番によって結果が変わる可能性があるので、変わらないように注意すること

    for (int i = 0; i < num; i++) {
#pragma omp parallel for
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                unsigned char* r = &pixels[(y * width + x) * bpp + 0];
                unsigned char* g = &pixels[(y * width + x) * bpp + 1];
                unsigned char* b = &pixels[(y * width + x) * bpp + 2];

                int cr = *r;
                int cg = *g;
                int cb = *b;
                int pixel_count = 1;
                // 左の色を加える
                if (0 < x) {
                    cr += *(r - bpp);
                    cg += *(g - bpp);
                    cb += *(b - bpp);
#pragma omp atomic
                    pixel_count++;
                }
                // 右の色を加える
                if (x < width - 1) {
                    cr += *(r + bpp);
                    cg += *(g + bpp);
                    cb += *(b + bpp);
#pragma omp atomic
                    pixel_count++;
                }
                // 上の色を加える
                if (0 < y) {
                    cr += *(r - width * bpp);
                    cg += *(g - width * bpp);
                    cb += *(b - width * bpp);
#pragma omp atomic
                    pixel_count++;
                }
                // 下の色を加える
                if (y < height - 1) {
                    cr += *(r + width * bpp);
                    cg += *(g + width * bpp);
                    cb += *(b + width * bpp);
#pragma omp atomic
                    pixel_count++;
                }
                // そのまま平均をとると桁落ちで暗くなるので、0.5だけ明るくする
                *r = (cr + pixel_count / 2) / pixel_count;
                *g = (cg + pixel_count / 2) / pixel_count;
                *b = (cb + pixel_count / 2) / pixel_count;
            }
        }
    }

    // 時間計測用：気にしないこと
    auto end = system_clock::now();
    std::cout << duration_cast<milliseconds>(end - start).count() << " milli sec. \n";

    // ファイルの保存
    stbi_write_png("blur.png", static_cast<int>(width), static_cast<int>(height), bpp, pixels, 0);
    // メモリの破棄
    stbi_image_free(pixels);

    return true;
}

int main()
{
    const char* filename = "two_sisters.png";

    monochrome(filename);
    blur(filename, 1000);

    return 0;
}
