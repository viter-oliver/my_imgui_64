// create_bmp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <iostream>
#include "BMPWriter.h"
#include <random>
    struct uv {
        int u : 16;
        int v : 16;
    };
int main(int argc,char** argv)
{

#if 0
    srand(time(NULL));

    std::vector<std::vector<std::vector<int> > > BMPImage;

    int w = 512;
    int h = 512;

    BMPImage.resize(w);

    for (int x = 0; x < w; x++) {
        BMPImage[x].resize(h - 1);
        BMPImage[x].push_back(std::vector<int>());
        for (int y = 0; y < h; y++) {
            BMPImage[x][y].push_back(255);
            BMPImage[x][y].push_back(0);
            BMPImage[x][y].push_back(0);
        }
    }

    BMPWriter::write_bmp("test.bmp", BMPImage);
#else
    using namespace BMPWriter;
    using namespace raw_image;
    using namespace std;
    img_raw BMPImage;

    int w = 1280;
    int h = 600;

    pixel ppu={90,200,200};
    BMPImage.resize(h, img_row(w));

    const int unit = 20;
    for (int x = 0; x < h; x++) {
        int rx = x % unit;
        if (x&&rx == 0) {
            ppu.r += 100;
        }
        for (int y = 0; y < w; y++) {
            //BMPImage[x][y].r=255-x;
            int ry = y  % unit;
            if (y&&ry == 0) {
                ppu.g += 100;
            }
            BMPImage[x][y] = ppu;
         }
        ppu.g = 100;
        
    }

    write_bmp2("test3.bmp", BMPImage);
    ofstream ofs;
    ofs.open("1280_600.raw");
    ofs.write((const char*)&BMPImage[0][0], h * w * 3);
    for (int i = 0; i < h; i++) {
        ofs.write((const char*)&BMPImage[i][0], w * 3);
    }
    ofs.close();
#endif
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
