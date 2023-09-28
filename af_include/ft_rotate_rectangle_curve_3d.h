#pragma once
#include "ft_base.h"
#include "af_shader.h"
#include "res_output.h"
#include "af_primitive_object.h"

namespace auto_future
{
	class AFG_EXPORT ft_rotate_rectangle_curve_3d :
          public ft_base
     {
          DEF_STRUCT_WITH_INIT( pty_page, _pt_tb,
            ( char, _attached_image[ FILE_NAME_LEN ] ),
            ( char, _attached_bg_image[FILE_NAME_LEN] ),
            ( float, _ori_x,{5.f}),
            ( float, _ori_z,{10.f}),
            ( float, _ra,{0}),
            ( float, _u_l, { 0.5f }),
            ( float, _width, { 1.f } ),
            ( float, _voffset, {0.f} ),
		    ( float, _near, { -1.f }),
		    ( float, _far, { -1.f }),
            ( af_vec3, _lane_clr ),
            ( af_vec3, _lane_bg_clr ))
          static ps_shader _phud_sd;
          static ps_primrive_object _ps_prm;
          ps_af_texture _pat_image;
          ps_af_texture _pat_bg_image;
     public:
          ft_rotate_rectangle_curve_3d();
          ~ft_rotate_rectangle_curve_3d();
          void link();
          void draw();
          ft_rotate_rectangle_curve_3d& set_near(float fValue) {
              _pt_tb._near = fValue;
              return *this;
          }
          ft_rotate_rectangle_curve_3d& set_far(float fValue) {
              _pt_tb._far = fValue;
              return *this;
          }
          ft_rotate_rectangle_curve_3d& set_ori_x(float fValue) {
              _pt_tb._ori_x = fValue;
              return *this;
          }
          ft_rotate_rectangle_curve_3d& set_ori_z(float fValue) {
              _pt_tb._ori_z = fValue;
              return *this;
          }
          ft_rotate_rectangle_curve_3d& set_rotate_angle(float fValue) {
              _pt_tb._ra = fValue;
              return *this;
          }
          ft_rotate_rectangle_curve_3d& set_lane_color(af_vec3 lane_col) {
              _pt_tb._lane_clr = lane_col;
              return *this;
          }
          ft_rotate_rectangle_curve_3d &set_lane_bg_color(af_vec3 lane_col) {
              _pt_tb._lane_bg_clr = lane_col;
              return *this;
          }
          ft_rotate_rectangle_curve_3d& set_voffset(float voffset) {
              _pt_tb._voffset = voffset;
              return *this;
          }
          void set_txt_obj(std::string txt_name) {
              strcpy(_pt_tb._attached_image, txt_name.c_str());
          }
          void set_txt_obj_bg(std::string txt_name) {
              strcpy(_pt_tb._attached_bg_image, txt_name.c_str());
          }
     };
     REGISTER_CONTROL( ft_rotate_rectangle_curve_3d );
}


