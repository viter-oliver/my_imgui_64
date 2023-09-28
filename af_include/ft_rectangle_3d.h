#pragma once
#include "af_primitive_object.h"
#include "af_shader.h"
#include "ft_base.h"
#include "res_output.h"
namespace auto_future {
class AFG_EXPORT ft_rectangle_3d : public ft_base {
  DEF_STRUCT_WITH_INIT(pty_page, _pt_tb, 
                       (float, _near, {-1.f}),
                       (float, _far, {-1.f}),
                       (char, _attached_image[FILE_NAME_LEN]),
                       (float,_alpha_nml,{1.f}),
                       (af_vec3,_model_clr))
  static ps_shader _prect_sd;
  static ps_primrive_object _ps_prm;
  ps_af_texture _pat_image;

public:
  ft_rectangle_3d();
  ~ft_rectangle_3d();
  void link();
  void draw();
  ft_rectangle_3d& set_model_col(af_vec3 md_col) {
      _pt_tb._model_clr = md_col;
      return *this;
  }
  ft_rectangle_3d& set_near(float fv) {
      _pt_tb._near = fv;
      return *this;
  }
  ft_rectangle_3d& set_far(float fv) {
      _pt_tb._far = fv;
      return *this;
  }
  ft_rectangle_3d& set_alpha(float alpha) {
    _pt_tb._alpha_nml = alpha;
    return *this;
  }
};
REGISTER_CONTROL(ft_rectangle_3d);
} // namespace auto_future