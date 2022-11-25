#pragma once
#include <map>
#include <string>
using namespace std;
struct texture_unit
{
	float _x0, _y0, _x1, _y1;
	texture_unit() :_x0(0), _y0(0), _x1(0), _y1(0){}
};
typedef map<string, texture_unit> mtxt_internal;
extern int g_txt_width_intl;
extern int g_txt_height_intl;
extern int g_txt_id_intl;
extern mtxt_internal g_mtxt_intl;
extern const char* icn_nm_file;
extern const char* icn_nm_feedback;
extern const char* icn_nm_shader;
extern const char* icn_nm_shader_source;
extern const char* icn_nm_material;
extern const char* icn_nm_texture;
extern const char* icn_nm_image;
extern const char* icn_nm_state_manager;
extern const char* icn_aliase;
extern const char* icn_shared_value;
extern const char* icn_primitive;
extern const char* icn_font;