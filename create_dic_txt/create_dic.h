#pragma once
#include "raw_image_def.h"

struct txt_u {
	unsigned short ih=0;
	unsigned short iv=0;
    bool operator == (txt_u& otxtu) {
        return ih == otxtu.ih && iv == otxtu.iv;
	}
};
using txt_u_row = std::vector<txt_u>;
using txt_dic = std::vector<txt_u_row>;
void create_dic(raw_image::img_raw& input_image, txt_dic& output_txt_dic);