#pragma once
#include "ft_base.h"
#include "af_shader.h"
#include "res_output.h"
#include "af_primitive_object.h"

namespace auto_future
{
	class AFG_EXPORT ft_3_times_road_curve_3d :
          public ft_base
     {
        DEF_STRUCT_WITH_INIT(pty_page, _pt_tb,
                             (char, _attached_image[FILE_NAME_LEN]),
                             (char, _attached_side_l_image[FILE_NAME_LEN]),
                             (char, _attached_side_r_image[FILE_NAME_LEN]),
                             (char, _attached_bg_image[FILE_NAME_LEN]),
                             (float, _coeff_hac[4]),
                             (float, _u_l, {0.f}),
                             (float, _base_z, {0.f}),
                             (float, _l_of, {0.f}),
                             (float, _r_of, {100.f}),
                             (float, _near, {-1.f}),
                             (float, _far, {-1.f}),
                             (float, _voffset, {0.f}),
                             (float, _side_voffset, {0.f}),
                             (float, _alpha_nml, {1.f}),
                             (float, _mix_nml, {1.f}),
                             (af_vec3, _lane_clr),
                             (af_vec3, _side_l_clr),
                             (af_vec3, _side_r_clr),
                             (af_vec3, _lane_bg_clr))

          static ps_shader _phud_sd;
          static ps_primrive_object _ps_prm;
          ps_af_texture _pat_image;
          ps_af_texture _pat_side_l_image;
          ps_af_texture _pat_side_r_image;
          ps_af_texture _pat_bg_image;
     public:
          ft_3_times_road_curve_3d();
          ~ft_3_times_road_curve_3d();
          void link();
          void draw();
          ft_3_times_road_curve_3d& set_near(float fValue) {
              _pt_tb._near = fValue;
              return *this;
          }
          ft_3_times_road_curve_3d& set_far(float fValue) {
              _pt_tb._far = fValue;
              return *this;
          }
          ft_3_times_road_curve_3d& set_coeff( float cf0, float cf1, float cf2, float cf3 )
          {
               _pt_tb._coeff_hac[ 0 ] = cf0;
               _pt_tb._coeff_hac[ 1 ] = cf1;
               _pt_tb._coeff_hac[ 2 ] = cf2;
               _pt_tb._coeff_hac[ 3 ] = cf3;
               return *this;
          }
          ft_3_times_road_curve_3d& set_lane_color(af_vec3 lane_col) {
              _pt_tb._lane_clr = lane_col;
              return *this;
          }
          ft_3_times_road_curve_3d &set_side_l_color(af_vec3 side_l_col) {
              _pt_tb._side_l_clr = side_l_col;
              return *this;
          }
          ft_3_times_road_curve_3d &set_side_r_color(af_vec3 side_r_col) {
              _pt_tb._side_r_clr = side_r_col;
              return *this;
          }
          ft_3_times_road_curve_3d &set_lane_bg_color(af_vec3 lane_bg_col) {
              _pt_tb._lane_bg_clr = lane_bg_col;
              return *this;
          }
          ft_3_times_road_curve_3d& set_voffset(float voffset) {
              _pt_tb._voffset = voffset;
              return *this;
          }
          ft_3_times_road_curve_3d &set_side_voffset(float voffset) {
              _pt_tb._side_voffset = voffset;
              return *this;
          }
          ft_3_times_road_curve_3d& set_base_z(float z) {
              _pt_tb._base_z = z;
              return *this;
          }
          void set_txt_obj(std::string txt_name) {
              strcpy(_pt_tb._attached_image, txt_name.c_str());
          }
          void set_txt_obj_side_l(std::string txt_name) {
              strcpy(_pt_tb._attached_side_l_image, txt_name.c_str());
          }

          void set_txt_obj_side_r(std::string txt_name) {
              strcpy(_pt_tb._attached_side_r_image, txt_name.c_str());
          }

          void set_txt_obj_bg(std::string txt_name) {
              strcpy(_pt_tb._attached_bg_image, txt_name.c_str());
          }
     };
     REGISTER_CONTROL( ft_3_times_road_curve_3d );
}


