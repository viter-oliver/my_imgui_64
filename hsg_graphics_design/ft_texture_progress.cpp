#include <cmath>
#include "ft_texture_progress.h"
#include "af_primitive_object.h"
#include "ft_light_scene.h"
#include "ft_trans.h"
#include "ft_raw_trans.h"
#include "af_shader_source_code.h"
namespace auto_future
{
	ps_shader ft_texture_progress::_ptxt_node_sd = nullptr;
     ft_texture_progress::ft_texture_progress()
     {
		 _pt._attached_txt[0] = '\0';
		 _pt._attached_obj[0] = '\0';
		 if (!ft_texture_progress::_ptxt_node_sd)
		 {
			 ft_texture_progress::_ptxt_node_sd = make_shared<af_shader>(modeling_vs_1_txt, modeling_fs_1_txt);
		 }
		 
#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)
		reg_property_handle(&_pt, 0, [this](void* member_address)
		{
			if (_ps_prm)
			{
				ImGui::Text("Attached primitive:%s", _pt._attached_obj);
				ImGui::SameLine();
				if (ImGui::Button("Delink##primitive"))
				{
					_ps_prm = nullptr;
				}
			}
			else
			{
			ImGui::InputText("Attached primitive:", _pt._attached_obj, FILE_NAME_LEN); 
			if (ImGui::Button("Import"))
			{
				auto itxt = g_primitive_list.find(_pt._attached_obj);
				if (itxt != g_primitive_list.end())
				{
					_ps_prm = itxt->second;
				}
			}
			}
		});
		reg_property_handle(&_pt, 1, [this](void* member_address)
		{
			if (_ps_txt)
			{
				ImGui::Text("Attached image:%s", _pt._attached_txt);
				ImGui::SameLine();
				if (ImGui::Button("Delink##txture"))
				{
					_ps_txt = nullptr;
				}
			}
			else
			{
				ImGui::InputText("Attached image:", _pt._attached_txt, FILE_NAME_LEN);
				if (ImGui::Button("Import##texture"))
				{
					auto itxt = g_mtexture_list.find(_pt._attached_txt);
					if (itxt != g_mtexture_list.end())
					{
						_ps_txt = itxt->second;
					}
				}
			}
		});
		reg_property_handle(&_pt, 2, [this](void* member_address) {
            ImGui::Combo("Draw mode:", &_pt._draw_mode, draw_mode,en_gl_count);
		});
#endif
     }

	 void ft_texture_progress::link(){
		 auto iprm = g_primitive_list.find(_pt._attached_obj);
		 if (iprm != g_primitive_list.end())
		 {
			 _ps_prm = iprm->second;
		 }
		 auto iat = g_mtexture_list.find(_pt._attached_txt);
		 if (iat != g_mtexture_list.end())
		 {
			 _ps_txt = iat->second;
		 }
	 }
	 void ft_texture_progress::draw(){
		 auto cnt = child_count();
		 for (int ix = 0; ix < cnt;++ix) {
			 auto pchild = get_child(ix);
			 pchild->draw();
		 }
		 if (_ps_prm == nullptr || _ps_txt==nullptr){
			 return;
		 }
		 glm::mat4 model;
		 auto pd=get_parent();
		 ft_light_scene* pscene = nullptr;
		 while (pd)
		 {
			 if (typeid(*pd) == typeid(ft_raw_trans)){
				 ft_raw_trans* pnode = static_cast <ft_raw_trans*>(pd);
				 pnode->transform(model);
			 } else if (typeid(*pd) == typeid(ft_light_scene)){
				 pscene = static_cast<ft_light_scene*>(pd);
				 break;
			 } else if (typeid(*pd) == typeid(ft_trans)){
				 ft_trans* pnode = static_cast<ft_trans*>(pd);
				 pnode->transform(model);
			 }
			 pd = pd->get_parent();
		 } 
		 
		 if (pscene == nullptr)
		 {
			 return;
		 }
		 //pscene->transform(model);
		 _ptxt_node_sd->use();
		 _ptxt_node_sd->uniform("model", glm::value_ptr(model));

		 auto pview = pscene->get_view_pos();
		 auto pcenter = pscene->get_center_of_prj();
		 auto pup = pscene->get_up();
		 glm::vec3 cam_pos(pview->x, pview->y, pview->z);
		 glm::vec3 cam_dir(pcenter->x, pcenter->y, pcenter->z);
		 glm::vec3 cam_up(pup->x, pup->y, pup->z);
		 glm::mat4 view = glm::lookAt(cam_pos, cam_dir, cam_up);
		 _ptxt_node_sd->uniform("view", glm::value_ptr(view));
		 _ptxt_node_sd->uniform("viewPos", glm::value_ptr(cam_pos));
			float aspect = pscene->get_aspect();
         float near_value = _pt._near > 0.f ? _pt._near : pscene->get_near();
         float far_value = _pt._far > 0.f ? _pt._far : pscene->get_far();
         glm::mat4 proj = glm::perspective(glm::radians(pscene->get_fovy()), aspect,near_value, far_value);
		 _ptxt_node_sd->uniform("projection", glm::value_ptr(proj));
		// auto& light_position = pscene->get_light_position();
		// _ptxt_node_sd->uniform("light_position", (float*)&light_position);
		 auto& light_dir = pscene->get_light_dir();
		 _ptxt_node_sd->uniform("light_dir", (float*)&light_dir);
		 auto& light_ambient = pscene->get_light_ambient();
		 _ptxt_node_sd->uniform("light_ambient", (float*)&light_ambient);
		 auto& light_diffuse = pscene->get_light_diffuse();
		 _ptxt_node_sd->uniform("light_diffuse", (float*)&light_diffuse);
		 auto& light_specular = pscene->get_light_specular();
		 _ptxt_node_sd->uniform("light_specular", (float*)&light_specular);
		 
		 //_ptxt_node_sd->uniform("col_ambient", (float*)&_pt._ambient_clr);
		 glActiveTexture(GL_TEXTURE0);
		 glBindTexture(GL_TEXTURE_2D, _ps_txt->_txt_id());
		 _ptxt_node_sd->uniform("texture_diffuse", 0);

		 glBindVertexArray(_ps_prm->_vao);
		 static GLuint draw_model[en_gl_count] = {
			 GL_POINTS,       
			 GL_LINES,     
			 GL_LINE_LOOP,
			 GL_LINE_STRIP,   
			 GL_TRIANGLES, 
			 GL_TRIANGLE_STRIP,
			 GL_TRIANGLE_FAN,
		 };		
		 GLuint dml = draw_model[_pt._draw_mode];
		 if (0 == _ps_prm->_ele_buf_len){
			 glDrawArrays(dml, 0, _ps_prm->_vertex_buf_len);
		 }
		 else {
			 glDrawElements(dml, _ps_prm->_ele_buf_len, GL_UNSIGNED_INT, 0);
		 }
	 }
}