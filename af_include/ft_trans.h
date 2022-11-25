#pragma once
#include "af_shader.h"
#include "ft_base.h"
namespace auto_future {
class AFG_EXPORT ft_trans : public ft_base {
  DEF_STRUCT_WITH_INIT(
      intl_pt, _pt, (int, _trans_order, {0}),
      (float, _trans_translation_x, {0.f}),
      (float, _trans_translation_y, {0.f}),
      (float, _trans_translation_z, {0.f}), 
      (float, _trans_scale_x, {1.f}),
      (float, _trans_scale_y, {1.f}), 
      (float, _trans_scale_z, {1.f}),
      (int, _rotate_order, {0}), 
      (float, _trans_rotation_x, {0.f}),
      (float, _trans_rotation_y, {0.f}), 
      (float, _trans_rotation_z, {0.f}))
public:
  ft_trans();
  void transform(glm::mat4 &model);
#define DEF_TRANS_SET(item,memb)\
  ft_trans &set_##item##_##memb (float fvalue) {\
     _pt._trans_##item##_##memb = fvalue;\
     return *this;\
  }
  DEF_TRANS_SET(translation,x)
  DEF_TRANS_SET(translation,y)
  DEF_TRANS_SET(translation,z)
  DEF_TRANS_SET(scale,x)
  DEF_TRANS_SET(scale,y)
  DEF_TRANS_SET(scale,z)
  DEF_TRANS_SET(rotation,x)
  DEF_TRANS_SET(rotation,y)
  DEF_TRANS_SET(rotation,z)
}; // namespace auto_future
REGISTER_CONTROL(ft_trans)
}
