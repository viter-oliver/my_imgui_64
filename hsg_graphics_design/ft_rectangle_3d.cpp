#include "ft_rectangle_3d.h"
#include "ft_light_scene.h"
#include "ft_trans.h"
static GLfloat vertices[] = {
    -0.5f, -0.5f, 0.f, 0.f, 1.f, 0.5f, -0.5f, 0.f, 1.f, 1.f,
    -0.5f, 0.5f,  0.f, 0.f, 0.f, 0.5f, 0.5f,  0.f, 1.f, 0.f,
};
const char *rect_vs = R"glsl(#version 300 es
precision mediump float;
layout(location=0) in vec3 position;
layout(location=1) in vec2 textCoord;
out vec2 TextCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    TextCoord = textCoord;
}
)glsl";
const char *rect_fs = R"glsl(#version 300 es
precision mediump float;
in vec2 TextCoord;
out vec4 o_clr;
uniform vec3 col_m;
uniform sampler2D text_at;
void main()
{
     vec4 base_col = texture(text_at, TextCoord);
     vec3 caucl_col = base_col.xyz * col_m;
	 o_clr = vec4(caucl_col,base_col.w);
}
)glsl";
namespace auto_future {
ps_shader ft_rectangle_3d::_prect_sd = nullptr;
ps_primrive_object ft_rectangle_3d::_ps_prm = nullptr;
ft_rectangle_3d::ft_rectangle_3d() {

  _in_p._sizew = _in_p._sizeh = 1000.f;

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)
  _pt_tb._attached_image[0] = '\0';
  reg_property_handle(&_pt_tb, 2, [this](void *member_address) {
    if (_pat_image) {
      ImGui::Text("Attached image:%s", _pt_tb._attached_image);
      ImGui::SameLine();
      if (ImGui::Button("Delink##attimage")) {
        _pat_image = nullptr;
      }
    } else {
      ImGui::InputText("Attached image:", _pt_tb._attached_image,
                       FILE_NAME_LEN);
      if (ImGui::Button("Import")) {
        auto itxt = g_mtexture_list.find(_pt_tb._attached_image);
        if (itxt != g_mtexture_list.end()) {
          _pat_image = itxt->second;
        }
      }
    }
  });

#endif
}

ft_rectangle_3d::~ft_rectangle_3d() {}

void ft_rectangle_3d::link() {
  auto iat = g_mtexture_list.find(_pt_tb._attached_image);
  if (iat != g_mtexture_list.end()) {
    _pat_image = iat->second;
  }

  if (!ft_rectangle_3d::_prect_sd) {
    ft_rectangle_3d::_prect_sd = make_shared<af_shader>(rect_vs, rect_fs);
    ft_rectangle_3d::_ps_prm = make_shared<primitive_object>();
    _ps_prm->set_ele_format({3, 2});
    _ps_prm->load_vertex_data(vertices, sizeof(vertices) / sizeof(float));
  }
}

void ft_rectangle_3d::draw() {
  if (!_pat_image) {
    return;
  }
  glm::mat4 model;
  auto pd = get_parent();
  ft_light_scene *pscene = nullptr;
  while (pd) {
    if (typeid(*pd) == typeid(ft_light_scene)) {
      pscene = static_cast<ft_light_scene *>(pd);
      break;
    } else if (typeid(*pd) == typeid(ft_trans)) {
      ft_trans *pnode = static_cast<ft_trans *>(pd);
      pnode->transform(model);
    }
    pd = pd->get_parent();
  }

  if (pscene == nullptr) {
    return;
  }

  
  af_vec3 *pview_pos = pscene->get_view_pos();
  af_vec3 *pcenter = pscene->get_center_of_prj();
  af_vec3 *pup = pscene->get_up();
  glm::vec3 cam_pos(pview_pos->x, pview_pos->y, pview_pos->z);
  glm::vec3 cam_dir(pcenter->x, pcenter->y, pcenter->z);
  glm::vec3 cam_up(pup->x, pup->y, pup->z);
  glm::mat4 view = glm::lookAt(cam_pos, cam_dir, cam_up);
  _prect_sd->use();
  _prect_sd->uniform("view", glm::value_ptr(view));
  float w, h;
  pscene->get_size(w, h);
  float aspect = w / h;
  float near_value = _pt_tb._near > 0.f ? _pt_tb._near : pscene->get_near();
  float far_value = _pt_tb._far > 0.f ? _pt_tb._far : pscene->get_far();

  glm::mat4 proj = glm::perspective(glm::radians(pscene->get_fovy()), aspect,
                                    near_value, far_value);
  _prect_sd->uniform("projection", glm::value_ptr(proj));
  _prect_sd->uniform("model", glm::value_ptr(model));
  _prect_sd->uniform("col_m",(float*)&_pt_tb._model_clr);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _pat_image->_txt_id());
  _prect_sd->uniform("text_at", 0);
  glBindVertexArray(_ps_prm->_vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

} // namespace auto_future