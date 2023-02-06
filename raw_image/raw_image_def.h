#pragma once
#include <vector>
namespace raw_image {
using BYTE = unsigned char;
using WORD = unsigned short;
using DWORD = unsigned long;
__pragma(pack(push, 1))
struct pixel {
        BYTE b={0}; 
        BYTE g={0};
        BYTE r={0};
};
__pragma(pack(pop))
using img_row = std::vector<pixel>;
using img_raw = std::vector<img_row>;
}
