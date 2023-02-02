#ifndef BMPWRITER_H 
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>

#include "raw_image_def.h"

namespace BMPWriter {
   
    void write_bmp(
        std::string filename,
        std::vector<std::vector<std::vector<int> > > BMPImage
    );
    
    void write_bmp2(
        std::string filename,
        raw_image::img_raw& BMPImage
    );
}

#endif
