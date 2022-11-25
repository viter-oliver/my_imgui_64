#pragma once
#include "ft_base.h"
#include "af_shader.h"
#include "af_primitive_object.h"
namespace auto_future
{
     class AFG_EXPORT ft_color_node :
          public ft_base
     {
          DEF_STRUCT_WITH_INIT( intl_pt, _pt,
			   (char, _attached_obj[FILE_NAME_LEN]),
               (float, _trans_mat4x4[16]),
			   (af_vec3, _ambient_clr),
			   (af_vec3, _diffuse_clr),
			   (af_vec3, _specular_clr))
		  static ps_shader _pcolor_node_sd;
		  ps_primrive_object _ps_prm = { nullptr };
     public:
		 ft_color_node();
#define DEF_COLOR_SET(item)\
	      ft_color_node&  set_##item(af_vec3 clr){\
			  _pt._##item##_clr=clr;\
			  return *this;\
		  }
		  DEF_COLOR_SET(ambient)
		  DEF_COLOR_SET(diffuse)
		  DEF_COLOR_SET(specular)
		  void set_trans_mat(float* pfvalue){
			  memcpy(_pt._trans_mat4x4, pfvalue, 16 * sizeof(float));
		  }
		  void set_prm_obj(std::string prm_name){
			  strcpy(_pt._attached_obj, prm_name.c_str());
		  }
      void transform( glm::mat4& model );
		  void link();
		  void draw();
     };
     REGISTER_CONTROL( ft_color_node )
}
