#pragma once
#include "ft_base.h"
#include "af_shader.h"
#include "af_primitive_object.h"
#include "res_output.h"

namespace auto_future
{
     class AFG_EXPORT ft_distortion :
          public ft_base
     {
          DEF_STRUCT_WITH_INIT( intl_pt, _pt,
			   (char, _attached_txt[FILE_NAME_LEN]))
		static ps_shader _pdis_sd;
		static ps_primrive_object _ps_prm;
		std::vector<float> px;
		std::vector<float> py;
		int be_valid = { 0 };
		float input_point[15][2], output_point[15][2];
		ps_af_texture _ps_txt = { nullptr };
     public:
		 ft_distortion();
		  void set_txt_obj(std::string txt_name){
			  strcpy(_pt._attached_txt, txt_name.c_str());
		  }
		  void link();
		  void draw();
     };
     REGISTER_CONTROL( ft_distortion )
}
