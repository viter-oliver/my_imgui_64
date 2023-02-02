#include "BMPWriter.h"

namespace BMPWriter {
    using namespace raw_image;

    /**
     * Write a BMP image.
     *
     * @param std::string filename,
     * @param std::vector<std::vector<std::vector<int>>> BMPImage - RGB + pixeldata
     */
    void write_bmp(
        std::string filename,
        std::vector<std::vector<std::vector<int>>> BMPImage
    ) {
        int h = BMPImage.size();
        int w = BMPImage[0].size();

        FILE* f;
        unsigned char* img = NULL;
        int filesize = 54 + 3 * w * h;

        if (img)
            free(img);

        img = (unsigned char*)malloc(3 * w * h);
        memset(img, 0, 3 * w * h);

        int x = 0;
        int y = 0;
        int xx = 0;
        int yy = 0;

        for (int xx = 0; xx < BMPImage.size(); xx++) {
            for (int yy = 0; yy < BMPImage[xx].size(); yy++) {
                if (BMPImage.size() > xx) {
                    if (BMPImage[xx].size() > yy) {
                        if (BMPImage[xx][yy].size() > 0) {
                            y = (h - 1) - yy;
                            img[(xx + y * w) * 3 + 2] = (unsigned char)(BMPImage[xx][yy][0]);
                            img[(xx + y * w) * 3 + 1] = (unsigned char)(BMPImage[xx][yy][1]);
                            img[(xx + y * w) * 3 + 0] = (unsigned char)(BMPImage[xx][yy][2]);
                        }
                    }
                }
            }
        }

        f = fopen(filename.c_str(), std::string("wb").c_str());

        unsigned char bmpfileheader[14] = {
            'B', 'M',       // bfType
            0, 0, 0, 0,     // bfSize
            0, 0,           // bfReserved1
            0, 0,           // bfReserved2
            54, 0, 0, 0     // bfOffBits
        };
        unsigned char bmpimageheader[40] = {
            40, 0, 0, 0,    // biSize
            0, 0, 0, 0,     // biWidth
            0, 0, 0, 0,     // biHeight
            1, 0,           // biPlanes
            24, 0           // biBitCount
        };
        unsigned char padding[3] = { 0,0,0 };

        bmpfileheader[2] = (unsigned char)(filesize);
        bmpfileheader[3] = (unsigned char)(filesize >> 8);
        bmpfileheader[4] = (unsigned char)(filesize >> 16);
        bmpfileheader[5] = (unsigned char)(filesize >> 24);

        bmpimageheader[4] = (unsigned char)(w);
        bmpimageheader[5] = (unsigned char)(w >> 8);
        bmpimageheader[6] = (unsigned char)(w >> 16);
        bmpimageheader[7] = (unsigned char)(w >> 24);
        bmpimageheader[8] = (unsigned char)(h);
        bmpimageheader[9] = (unsigned char)(h >> 8);
        bmpimageheader[10] = (unsigned char)(h >> 16);
        bmpimageheader[11] = (unsigned char)(h >> 24);

        fwrite(bmpfileheader, 1, 14, f);
        fwrite(bmpimageheader, 1, 40, f);

        for (int i = 0; i < h; i++) {
            fwrite(img + (w * (h - i - 1) * 3), 3, w, f);
            fwrite(padding, 1, (4 - (w * 3) % 4) % 4, f);
        }

        fclose(f);
    }

    __pragma(pack(push, 1))
    struct BITMAPFILEHEADER {
        WORD bfType;
        DWORD bfSize;
        WORD bfRreserved1;
        WORD bfRreserved2;
        DWORD bfOffBits;
    };
 
    struct BITMAPINFOHEADER {
        DWORD biSize;
        long biWidth;
        long biHeight;
        WORD biPlanes;
        WORD biBitCount;
        DWORD biCompression;
        DWORD biSizeImage;
        long biXpelsPerMeter;
        long biYpelsPerMeter;
        DWORD biClrUsed;
        DWORD biClrImportant;
    };
   __pragma(pack(pop))

    void write_bmp2(
        std::string filename,
        raw_image::img_raw& BMPImage
    ) {
        const int h = BMPImage.size();
        const int w = BMPImage[0].size();
        const int size = h * w * 3;
        BITMAPFILEHEADER fileHeader={
            0x4D42,
            sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+size,
            0,0,
            sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)
        };
        BITMAPINFOHEADER bitmapHeader = {0};
        bitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapHeader.biHeight = h;
        bitmapHeader.biWidth = w;
        bitmapHeader.biPlanes = 1;
        bitmapHeader.biBitCount = 24;
        bitmapHeader.biSizeImage = size;
        FILE* f = fopen(filename.c_str(), std::string("wb").c_str());
        auto bh = sizeof(BITMAPFILEHEADER);
        auto bi = sizeof(BITMAPINFOHEADER);
        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, f);
        fwrite(&bitmapHeader, sizeof(BITMAPINFOHEADER), 1, f);
        for (int i = 0; i < h; i++) {
            fwrite(&BMPImage[i][0], 3, w, f);
        }
        //fwrite(&BMPImage[0][0], size, 1, f);

       
        fclose(f);
    }
}