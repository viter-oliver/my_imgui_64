#pragma once
#include "ft_base.h"
#include "af_shader.h"
#include "res_output.h"
#include "af_primitive_object.h"

namespace auto_future
{
	class AFG_EXPORT ft_4_time_curve_3d :
          public ft_base
     {
          DEF_STRUCT_WITH_INIT( pty_page, _pt_tb,
                                ( char, _attached_image[ FILE_NAME_LEN ] ),
                                ( float, _coeff_hac[4] ),
                                ( bool, _left_border, {true} ),
                                ( float, _width, { 100.f } ),
								( float, _near, { -1.f }),
								( float, _far, { -1.f }),
                                ( float, _tanslation_x, { 0.f } ),
                                ( float, _tanslation_y, { 0.f } ),
                                ( float, _tanslation_z, { 0.f } ),
                                (float,_voffset,{0.f}),
                                (af_vec3,_lane_clr))
          static ps_shader _phud_sd;
          static ps_primrive_object _ps_prm;
          ps_af_texture _pat_image;
     public:
          ft_4_time_curve_3d();
          ~ft_4_time_curve_3d();
          void link();
          void draw();
          void set_transx( float transx )
          {
               _pt_tb._tanslation_x = transx;
          }
          void set_transy( float transy )
          {
               _pt_tb._tanslation_y = transy;
          }
          void set_transz( float transz )
          {
               _pt_tb._tanslation_z = transz;
          }
          ft_4_time_curve_3d& set_near(float fValue) {
              _pt_tb._near = fValue;
              return *this;
          }
          ft_4_time_curve_3d& set_far(float fValue) {
              _pt_tb._far = fValue;
              return *this;
          }
          ft_4_time_curve_3d& set_coeff( float cf0, float cf1, float cf2, float cf3 )
          {
               _pt_tb._coeff_hac[ 0 ] = cf0;
               _pt_tb._coeff_hac[ 1 ] = cf1;
               _pt_tb._coeff_hac[ 2 ] = cf2;
               _pt_tb._coeff_hac[ 3 ] = cf3;
               return *this;
          }
          ft_4_time_curve_3d& set_lane_color(af_vec3 lane_col) {
              _pt_tb._lane_clr = lane_col;
              return *this;
          }
          ft_4_time_curve_3d& set_voffset(float voffset) {
              _pt_tb._voffset = voffset;
              return *this;
          }
          void set_txt_obj(std::string txt_name) {
              strcpy(_pt_tb._attached_image, txt_name.c_str());
          }
     };
     REGISTER_CONTROL( ft_4_time_curve_3d );
}


