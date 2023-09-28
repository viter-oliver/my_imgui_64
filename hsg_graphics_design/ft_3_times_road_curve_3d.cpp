#include "ft_3_times_road_curve_3d.h"
#include "ft_light_scene.h"
#include "ft_trans.h"
static const char* sd_4_curve_vs = R"glsl(#version 300 es
precision highp float;
layout(location=0) in vec3 position;
layout(location=1) in vec2 textCoord;
out vec2 TextCoord;
out vec2 BgTextCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float c[4];
uniform float u_l;
uniform float base_z;
uniform float l_of;
uniform float r_of;
uniform float voffset;
void main()
{
    vec3 pos=position;
    float posx0=pos.x;
    float z=pos.z * u_l + base_z;
    pos.z=z;
    if(posx0<0.1)
    {
        pos.x=c[3]*z*z*z+c[2]*z*z+c[1]*z+c[0]-l_of;
    }
    else
    {
        pos.x=c[3]*z*z*z+c[2]*z*z+c[1]*z+c[0]+r_of;
    }
    gl_Position = projection * view * model * vec4(pos, 1.0);
    TextCoord = vec2(textCoord.x,textCoord.y + voffset);
    BgTextCoord = textCoord;
}
)glsl";
static const char* sd_4_curve_fs = R"glsl(#version 300 es
precision highp float;
in vec2 TextCoord;
in vec2 BgTextCoord;
out vec4 o_clr;
uniform vec3 lane_color;
uniform vec3 lane_bg_color;
uniform float alpha;
uniform sampler2D text_at;
uniform sampler2D bg_text_at;
void main()
{
    vec4 base_col = texture(text_at, TextCoord);
    vec4 bg_col = texture(bg_text_at, BgTextCoord);
    base_col.xyz = base_col.xyz * lane_color;
    bg_col.xyz = bg_col.xyz * lane_bg_color;
    vec4 t_clr = mix(base_col,bg_col,0.8);
    t_clr.a = t_clr.a*alpha;
    o_clr =t_clr;
}
)glsl";

namespace auto_future
{
     ps_shader ft_3_times_road_curve_3d::_phud_sd = nullptr;
     ps_primrive_object ft_3_times_road_curve_3d::_ps_prm = nullptr;

     ft_3_times_road_curve_3d::ft_3_times_road_curve_3d()
     {
          /*if( !_phud_sd )
          {
               _phud_sd = make_shared<af_shader>( sd_4_curve_vs, sd_4_curve_fs );
          }
        */

          _pt_tb._attached_image[ 0 ] = '\0';
          _pt_tb._lane_clr = { 1.f,1.f,1.f };
          _pt_tb._coeff_hac[ 0 ] = _pt_tb._coeff_hac[ 1 ] = _pt_tb._coeff_hac[ 2 ] = _pt_tb._coeff_hac[ 3 ] = 0.f;
#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)
          reg_property_handle( &_pt_tb, 0, [this]( void* member_address )
          {
               if( _pat_image )
               {
                    ImGui::Text( "Attached image:%s", _pt_tb._attached_image );
                    ImGui::SameLine();
                    if( ImGui::Button( "Delink##attimage" ) )
                    {
                         _pat_image = nullptr;
                    }
               }
               else
               {
                    ImGui::InputText( "Attached image:", _pt_tb._attached_image, FILE_NAME_LEN );
                    if( ImGui::Button( "Import" ) )
                    {
                         auto itxt = g_mtexture_list.find( _pt_tb._attached_image );
                         if( itxt != g_mtexture_list.end() )
                         {
                              _pat_image = itxt->second;
                         }
                    }
               }
          } );
          reg_property_handle(&_pt_tb, 1, [this](void *member_address) {
              if (_pat_bg_image) {
                  ImGui::Text("Attached bg image:%s", _pt_tb._attached_bg_image);
                  ImGui::SameLine();
                  if (ImGui::Button("Delink##attbgimage")) {
                      _pat_bg_image = nullptr;
                  }
              } else {
                  ImGui::InputText("Attached bg image:", _pt_tb._attached_bg_image, FILE_NAME_LEN);
                  if (ImGui::Button("Import##bg")) {
                      auto itxt = g_mtexture_list.find(_pt_tb._attached_bg_image);
                      if (itxt != g_mtexture_list.end()) {
                          _pat_bg_image = itxt->second;
                      }
                  }
              }
          });


#endif
     }

     ft_3_times_road_curve_3d::~ft_3_times_road_curve_3d()
     {

     }
     const int curve_len = 200;
     const int point_cnt = curve_len * 2 + 2;

     void ft_3_times_road_curve_3d::link()
     {
          auto iat = g_mtexture_list.find( _pt_tb._attached_image );
          if( iat != g_mtexture_list.end() )
          {
               _pat_image = iat->second;
          }
          auto bgat = g_mtexture_list.find(_pt_tb._attached_bg_image);
          if (bgat != g_mtexture_list.end()) {
              _pat_bg_image = bgat->second;
          }
          if( !ft_3_times_road_curve_3d::_phud_sd )
          {
              ft_3_times_road_curve_3d::_phud_sd = make_shared<af_shader>( sd_4_curve_vs, sd_4_curve_fs );
              ft_3_times_road_curve_3d::_ps_prm = make_shared<primitive_object>();
              int demension = 5;
              auto data_cnt = point_cnt*demension;
              GLfloat* vertices = new GLfloat[ data_cnt ];
              float uv_unit = 1.f / (float)curve_len;
              float zmax = 0;
              for( int ix = 0; ix < curve_len + 1; ++ix )
              {
                   auto base_id = ix * 2 * demension;
                   vertices[ base_id ] = 0;//x->
                   vertices[ base_id + 1 ] = 0;//y->
                   vertices[ base_id + 2 ] = ix;//z->
                   vertices[ base_id + 3 ] = 0;
                   vertices[ base_id + 4 ] = uv_unit * ix;

                   vertices[ base_id + 5 ] = 1;//x->
                   vertices[ base_id + 6 ] = 0;//y->
                   vertices[ base_id + 7 ] = ix;//z->
                   zmax = vertices[base_id + 7];
                   vertices[ base_id + 8 ] = 1;
                   vertices[ base_id + 9 ] = uv_unit * ix;
              }

              ft_3_times_road_curve_3d::_ps_prm->set_ele_format( { 3, 2 } );
              ft_3_times_road_curve_3d::_ps_prm->load_vertex_data( vertices, data_cnt );
              delete[] vertices;
          }
     }

     void ft_3_times_road_curve_3d::draw()
     {
         if (!_pat_image || !_pat_bg_image) {
             return;
         }
          glm::mat4 model;
          auto pd = get_parent();
          ft_light_scene* pscene = nullptr;
          while (pd) {
              if (typeid(*pd) == typeid(ft_light_scene)) {
                  pscene = static_cast<ft_light_scene*>(pd);
                  break;
              }
              else if (typeid(*pd) == typeid(ft_trans)) {
                  ft_trans* pnode = static_cast<ft_trans*>(pd);
                  pnode->transform(model);
              }
              pd = pd->get_parent();
          }
          af_vec3* pview_pos = pscene->get_view_pos();
          af_vec3* pcenter = pscene->get_center_of_prj();
          af_vec3* pup = pscene->get_up();
          glm::vec3 cam_pos( pview_pos->x, pview_pos->y, pview_pos->z );
          glm::vec3 cam_dir( pcenter->x, pcenter->y, pcenter->z );
          glm::vec3 cam_up( pup->x, pup->y, pup->z );
          glm::mat4 view = glm::lookAt( cam_pos, cam_dir, cam_up );
          _phud_sd->use();
          _phud_sd->uniform( "view", glm::value_ptr( view ) );
          float aspect = pscene->get_aspect();
		  float near_value = _pt_tb._near>0.f ? _pt_tb._near : pscene->get_near();
		  float far_value = _pt_tb._far>0.f ? _pt_tb._far : pscene->get_far();

          glm::mat4 proj = glm::perspective( glm::radians(pscene->get_fovy() ), aspect, near_value, far_value );
          _phud_sd->uniform( "projection", glm::value_ptr( proj ) );
          
          _phud_sd->uniform("model", glm::value_ptr(model));
          _phud_sd->uniform( "c[0]", _pt_tb._coeff_hac );
          _phud_sd->uniform( "u_l", &_pt_tb._u_l);
          _phud_sd->uniform( "base_z", &_pt_tb._base_z );
          _phud_sd->uniform("l_of", &_pt_tb._l_of);
          _phud_sd->uniform("r_of", &_pt_tb._r_of);
          _phud_sd->uniform("voffset", &_pt_tb._voffset);
          _phud_sd->uniform("alpha", &_pt_tb._alpha_nml);
          _phud_sd->uniform("lane_color", (float*) & _pt_tb._lane_clr);
          _phud_sd->uniform("lane_bg_color", (float *)&_pt_tb._lane_bg_clr);
          glActiveTexture( GL_TEXTURE0 );
          glBindTexture( GL_TEXTURE_2D, _pat_image->_txt_id() );
          _phud_sd->uniform( "text_at", 0 );
          glActiveTexture(GL_TEXTURE1);
          glBindTexture(GL_TEXTURE_2D, _pat_bg_image->_txt_id());
          _phud_sd->uniform("bg_text_at", 1);
          glBindVertexArray( _ps_prm->_vao );
          glDrawArrays( GL_TRIANGLE_STRIP, 0, point_cnt );
     }
}