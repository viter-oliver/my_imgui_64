#include <cmath>
#include "ft_simple_mesh.h"
#include "af_primitive_object.h"
#include "ft_light_scene.h"
#include "ft_trans.h"
#include "ft_raw_trans.h"
#include "af_shader_source_code.h"

namespace auto_future
{
	ps_shader ft_simple_mesh::_ptxt_node_sd = nullptr;
     ft_simple_mesh::ft_simple_mesh()
     {
		 _pt._attached_txt[0] = '\0';
		 _pt._attached_obj[0] = '\0';
		 if (!ft_simple_mesh::_ptxt_node_sd)
		 {
			 ft_simple_mesh::_ptxt_node_sd = make_shared<af_shader>(single_txt_vs, single_txt_fs);
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
#endif
     }

	 void ft_simple_mesh::link(){
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
	 void ft_simple_mesh::draw(){
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
		 _ptxt_node_sd->uniform("voffset", &_pt._voffset);
		 _ptxt_node_sd->uniform("model", glm::value_ptr(model));

		 auto pview = pscene->get_view_pos();
		 auto pcenter = pscene->get_center_of_prj();
		 auto pup = pscene->get_up();
		 glm::vec3 cam_pos(pview->x, pview->y, pview->z);
		 glm::vec3 cam_dir(pcenter->x, pcenter->y, pcenter->z);
		 glm::vec3 cam_up(pup->x, pup->y, pup->z);
		 glm::mat4 view = glm::lookAt(cam_pos, cam_dir, cam_up);
		 _ptxt_node_sd->uniform("view", glm::value_ptr(view));
		 float w, h;
		 pscene->get_size(w, h);
		 float aspect = w / h;
		 glm::mat4 proj = glm::perspective(glm::radians(pscene->get_fovy()), aspect, pscene->get_near(), pscene->get_far());
		 _ptxt_node_sd->uniform("projection", glm::value_ptr(proj));
		// auto& light_position = pscene->get_light_position();
		// _ptxt_node_sd->uniform("light_position", (float*)&light_position);
		 
		 
		 //_ptxt_node_sd->uniform("col_ambient", (float*)&_pt._ambient_clr);
		 glActiveTexture(GL_TEXTURE0);
		 glBindTexture(GL_TEXTURE_2D, _ps_txt->_txt_id());
		 _ptxt_node_sd->uniform("tex1", 0);

		 glBindVertexArray(_ps_prm->_vao);
		 if (0 == _ps_prm->_ele_buf_len){
			 glDrawArrays(GL_TRIANGLES, 0, _ps_prm->_vertex_buf_len);
		 }
		 else {
			 glDrawElements(GL_TRIANGLES, _ps_prm->_ele_buf_len, GL_UNSIGNED_INT, 0);
		 }
	 }
}