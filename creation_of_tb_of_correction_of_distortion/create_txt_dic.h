#ifndef CREATE_TXT_DIC_H
#define CREATE_TXT_DIC_H
#include <string>
#include <functional>
#include "valves.hpp"
void create_txt_dic(std::string& img_file,int txt_width,int txt_height,int block_size, std::vector<txt_dic::dic_uint>& txt_dic_o,std::function<void(int)> progress );
#endif