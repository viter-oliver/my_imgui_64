#include "ft_rotate_curve_3d.h"
#include "ft_light_scene.h"
#include "ft_trans.h"
#include "ft_raw_trans.h"

static const char* sd_rt_curve_vs = R"glsl(#version 300 es
precision highp float;
layout(location=0) in vec3 position;
layout(location=1) in vec2 textCoord;
out vec2 TextCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float ori_x;
uniform float ori_z;
uniform float ra;
uniform float w;
uniform float voffset;
const float right_a=1.707963;//pi*0.5
void main()
{
    vec3 pos=position;
    float posx0=pos.x;
    float z=pos.z;//index
    if(ori_x>0.1){//right rotate
        float rd_in=ori_x;
        float rd_out=rd_in+w;
        float r_s=rd_in*ra;
        float da=ra/r_s;
        if( z<=ori_z ) {
            if(posx0<0.1){//left
              pos.x=-w;
            } else {
              pos.x=0.f;
            }
        } else if(z>ori_z&&z<=(ori_z+r_s)) {
            float tra=(z-ori_z)*da;
            if(posx0<0.1){//left
              pos.x=rd_out*(1.0f-cos(tra))-w;
              pos.z=rd_out*sin(tra)+ori_z;
            } else {
              pos.x=rd_in*(1.0f-cos(tra));
              pos.z=rd_in*sin(tra)+ori_z;
            }
        } else {
            float z_span=z-r_s-ori_z;
            float ta=right_a-ra;
            float dx=z_span*cos(ta);
            float dz=z_span*sin(ta);
            if(posx0<0.1){//left
               float pos_out_x0=rd_out*(1.0f-cos(ra))-w;
               float pos_out_z0=rd_out*sin(ra)+ori_z;
               pos.x=pos_out_x0+dx;
               pos.z=pos_out_z0+dz;
            } else {
               float pos_in_x0=rd_in*(1.0f-cos(ra));
               float pos_in_z0=rd_in*sin(ra)+ori_z;
               pos.x= pos_in_x0+dx;
               pos.z= pos_in_z0+dz;
              
            }
        }
    } else { //left rotate
        float rd_in=-ori_x;
        float rd_out=rd_in+w;
        float r_s=rd_in*ra;
        float da=ra/r_s;
        if( z<=ori_z ) {
            if(posx0<0.1){//left
              pos.x=0.f;
            } else {
              pos.x=w;
            }
        } else if(z>ori_z&&z<=(ori_z+r_s)) {
            float tra=(z-ori_z)*da;
            if(posx0<0.1){//left
              pos.x=rd_in*(cos(tra)-1.0f);
              pos.z=rd_in*sin(tra)+ori_z;
            } else {
              pos.x=w-rd_out*(1.0f-cos(tra));
              pos.z=rd_out*sin(tra)+ori_z;
            }
        } else {//TODO:
            float z_span=z-r_s-ori_z;
            float ta=right_a-ra;
            float dx=-z_span*cos(ta);
            float dz=z_span*sin(ta);
            if(posx0<0.1){//left
               float pos_in_x0=-rd_in*(1.0f-cos(ra));
               float pos_in_z0=rd_in*sin(ra)+ori_z;
               pos.x= pos_in_x0+dx;
               pos.z= pos_in_z0+dz;
            } else {              
               float pos_out_x0=w-rd_out*(1.0f-cos(ra));
               float pos_out_z0=rd_out*sin(ra)+ori_z;
               pos.x=pos_out_x0+dx;
               pos.z=pos_out_z0+dz;
            }
        }
    }
    gl_Position = projection * view * model * vec4(pos, 1.0);
    TextCoord = vec2(textCoord.x,textCoord.y + voffset);
}
)glsl";
static const char* sd_rt_curve_fs = R"glsl(#version 300 es
precision highp float;
in vec2 TextCoord;
out vec4 o_clr;
uniform vec3 lane_color;
uniform sampler2D text_at;
void main()
{
    vec4 base_col = texture(text_at, TextCoord);
    vec3 caucl_col = base_col.xyz * lane_color;
    o_clr = vec4(caucl_col,base_col.w);
}
)glsl";

namespace auto_future
{
     ps_shader ft_rotate_curve_3d::_phud_sd = nullptr;
     ps_primrive_object ft_rotate_curve_3d::_ps_prm = nullptr;

     ft_rotate_curve_3d::ft_rotate_curve_3d()
     {
          /*if( !_phud_sd )
          {
               _phud_sd = make_shared<af_shader>( sd_rt_curve_vs, sd_rt_curve_fs );
          }
        */

          _pt_tb._attached_image[ 0 ] = '\0';
          _pt_tb._lane_clr = { 1.f,1.f,1.f };
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
#endif
     }

     ft_rotate_curve_3d::~ft_rotate_curve_3d()
     {

     }
     const int curve_len = 100;
     const int point_cnt = curve_len * 2 + 2;

     void ft_rotate_curve_3d::link()
     {
          auto iat = g_mtexture_list.find( _pt_tb._attached_image );
          if( iat != g_mtexture_list.end() )
          {
               _pat_image = iat->second;
          }         
          if( !ft_rotate_curve_3d::_phud_sd )
          {
              ft_rotate_curve_3d::_phud_sd = make_shared<af_shader>( sd_rt_curve_vs, sd_rt_curve_fs );
              ft_rotate_curve_3d::_ps_prm = make_shared<primitive_object>();
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
                   vertices[ base_id + 4 ] = ix;

                   vertices[ base_id + 5 ] = 1;//x->
                   vertices[ base_id + 6 ] = 0;//y->
                   vertices[ base_id + 7 ] = ix;//z->
                   zmax = vertices[base_id + 7];
                   vertices[ base_id + 8 ] = 1;
                   vertices[ base_id + 9 ] = uv_unit * ix;
              }

              ft_rotate_curve_3d::_ps_prm->set_ele_format( { 3, 2 } );
              ft_rotate_curve_3d::_ps_prm->load_vertex_data( vertices, data_cnt );
              delete[] vertices;
          }
     }

     void ft_rotate_curve_3d::draw()
     {
          if( !_pat_image )
          {
               return;
          }
          glm::mat4 model;
          auto pd = get_parent();
          ft_light_scene* pscene = nullptr;
          while (pd)
          {
              if (typeid(*pd) == typeid(ft_raw_trans)) {
                  ft_raw_trans* pnode = static_cast <ft_raw_trans*>(pd);
                  pnode->transform(model);
              }
              else if (typeid(*pd) == typeid(ft_light_scene)) {
                  pscene = static_cast<ft_light_scene*>(pd);
                  break;
              }
              else if (typeid(*pd) == typeid(ft_trans)) {
                  ft_trans* pnode = static_cast<ft_trans*>(pd);
                  pnode->transform(model);
              }
              pd = pd->get_parent();
          }

          if (pscene == nullptr)
          {
              return;
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
          float w, h;
          pscene->get_size( w, h );
          float aspect = w / h;
		  float near_value = _pt_tb._near>0.f ? _pt_tb._near : pscene->get_near();
		  float far_value = _pt_tb._far>0.f ? _pt_tb._far : pscene->get_far();

          glm::mat4 proj = glm::perspective( glm::radians(pscene->get_fovy() ), aspect, near_value, far_value );
          _phud_sd->uniform( "projection", glm::value_ptr( proj ) );
          _phud_sd->uniform( "model", glm::value_ptr( model ) );
          _phud_sd->uniform("ori_x", &_pt_tb._ori_x);
          _phud_sd->uniform("ori_z", &_pt_tb._ori_z);
          _phud_sd->uniform("ra", &_pt_tb._ra);
          _phud_sd->uniform( "w", &_pt_tb._width );
          _phud_sd->uniform("voffset", &_pt_tb._voffset);
          _phud_sd->uniform("lane_color", (float*) & _pt_tb._lane_clr);
          glActiveTexture( GL_TEXTURE0 );
          glBindTexture( GL_TEXTURE_2D, _pat_image->_txt_id() );
          _phud_sd->uniform( "text_at", 0 );
          glBindVertexArray( _ps_prm->_vao );
          glDrawArrays( GL_TRIANGLE_STRIP, 0, point_cnt );
     }
}