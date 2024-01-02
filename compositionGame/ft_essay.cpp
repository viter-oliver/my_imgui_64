#include "ft_essay.h"
#include "ft_sentence.h"
#include "ft_paragraph.h"
#include <random>
#include <algorithm>
#include <cmath>
using namespace std;
namespace auto_future
{
     ft_essay::ft_essay()
     {}


     ft_essay::~ft_essay()
     {}
     static default_random_engine generator;
     static uniform_real_distribution<float> distribution( 0.5, 1.f );

     int ft_essay::load_content( wstring& str_content )
     {
          game_state = "";
          if( !_pfont_unit ||!str_content.size())
          {
               return;
          }
          for( auto it_child = _vchilds.begin(); it_child != _vchilds.end(); )
          {
               auto pchild = *it_child;
               delete pchild;
               it_child = _vchilds.erase( it_child );
          }
          wstring wsparagraph;
          auto pnew_paragraph = new ft_paragraph();

          int chd_cnt = 0, chd_pcnt = 0;
          auto add_sentaence_2_paragraph = [&] {
            auto pnew_sentence = new ft_sentence(_pfont_unit, wsparagraph);
            float r = distribution(generator);
            float g = distribution(generator);
            float b = distribution(generator);

            pnew_sentence->set_txt_clr(r, g, b, 1.f);
            pnew_sentence->load_idx = chd_cnt++;
            pnew_paragraph->add_child(pnew_sentence);
            wsparagraph.clear();
          };
          auto set_paragraph_border_col = [&] {
            float r = distribution(generator);
            float g = distribution(generator);
            float b = distribution(generator);
            pnew_paragraph->set_border_color(r, g, b, 1);
          };
          srand( unsigned( time( 0 ) ) );
          for (int ix = 0; ix < str_content.size();ix++) {
            auto& ch = str_content[ix];
            if (ch == 0x3002) {
              wsparagraph += ch;
              if (ix < str_content.size() - 1 &&
                  str_content[ix + 1] == 0x201d) {
                wsparagraph += str_content[ix + 1];
                ix++;
              }
              add_sentaence_2_paragraph();
            } else if (ch == 0xa) {
              if (wsparagraph.size() > 0) {
                add_sentaence_2_paragraph();
              }
              chd_cnt = 0;
              pnew_paragraph->load_idx = chd_pcnt++;
              set_paragraph_border_col();
              add_child(pnew_paragraph);
              pnew_paragraph = new ft_paragraph();
            } else if (ch !='\r') {
              wsparagraph += ch;
            }
          }
          if (pnew_paragraph->get_child_count() > 0) {
            pnew_paragraph->load_idx = chd_pcnt;
            set_paragraph_border_col();
            add_child(pnew_paragraph);
          } else {
            delete pnew_paragraph;
          }
          _cur_order.paragraphs_order.assign(_vchilds.size(), true);
          _cur_order.paragraphs_restored = true;
          auto unit_cnt = chd_cnt * chd_pcnt;
          sec_of_pass_score = unit_cnt * 30;
          sec_of_superb_score = sec_of_pass_score * 0.6;
          sec_of_mid_score = sec_of_pass_score * 0.8;
          sec_of_prepare = sec_of_pass_score * 0.04;
          _load_start= steady_clock::now();
          return unit_cnt;
          
     }
     af_vec4 finish_state_col[en_score_cnt] = {
       {0.74f,0.16f,0.8f,1.f},
       {0.f,0.f,1.f,1.f},
       {1.f,0.f,0.f,1.f},
       {1.f,0.f,0.f,1.f}
     };
     void ft_essay::draw()
     {
          const int dur_show = 7853;
          if (!_pfont_unit)
          {
               return;
          }
         
          auto currentTime = steady_clock::now();
          if (!_gaming) {
            if (sec_of_prepare > 0) {//?y?¨²?¡è¨¤¨¤
              auto load_its = duration_cast<seconds>(currentTime - _load_start);
              auto load_sec = load_its.count();
              sec_of_prepare -= load_sec;
              if (sec_of_prepare <= 0) {

              }
              shuffle();
            } else {//¨®??¡¤¨°??-?¨¢¨º?
              auto its = duration_cast<milliseconds>(currentTime - _finish_start);
              auto finish_consume = its.count();
              if (finish_consume > dur_show) {//??¡ä£¤¡¤¡é??¨°???¨®??¡¤
                game_triger(game_score);
              } else {
                ImVec2 abpos = absolute_coordinate_of_base_pos();
                ImVec2 winpos = ImGui::GetWindowPos();
                ImVec2 dpos = abpos + winpos;
                af_vec2 draw_pos = { dpos.x, dpos.y };
                af_vec2 _endpos;
                float sc = 3 * cos(finish_consume / 1000.f);
                g_pfont_face_manager->draw_wstring(_pfont_unit, font_size,
                  draw_pos, _endpos,
                  sc, game_finish_state,
                  finish_state_col[game_score], 1000.f,
                  0, false);
              }
              return;

            }
            
          }
          auto its = duration_cast<seconds>(currentTime - _play_start);
          consume_seconds=its.count();
          //consume_seconds = chrono::duration_cast<chrono::duration<seconds>>(currentTime - _play_start).count().count();

          if (consume_seconds >= sec_of_pass_score) {
            game_finish_state = L"you loss the game!";
            game_score = en_fail;
            _gaming = false;
            _finish_start= steady_clock::now();
          }

          auto wsz = ImGui::GetWindowSize();
          auto winpos = ImGui::GetWindowPos();
          auto l_edge = hmargin+winpos.x;
          auto r_edge = wsz.x - hmargin+winpos.x;
          af_vec2 draw_pos = {hmargin,vmargin};
          auto para_width = r_edge - l_edge;
          int ip = 0;
          auto font_height=font_size + line_spacing;
          for (auto& ipa:_vchilds) {
            ft_paragraph* ppara = static_cast<ft_paragraph*>(ipa);
            ppara->set_base_pos(draw_pos.x, draw_pos.y);
            af_vec2 spos = {20, 20};
            float p_bottom = 0;
            for (int ix = 0; ix < ppara->get_child_count(); ix++) {
                ft_sentence* pstc = static_cast<ft_sentence*>(ppara->get_child(ix));
			          pstc->set_font_size(font_size);
                pstc->init_edge( l_edge, r_edge );
                pstc->line_spacing = line_spacing;
                pstc->set_base_pos( spos.x, spos.y );
                pstc->draw();
                spos.x = pstc->_endpos.x-winpos.x+_in_p._posx;
                if (!pstc->is_same_line())
                {
                  spos.y += pstc->delta_height;  // +_in_p._posy;
                }
                p_bottom = pstc->_endpos.y;
            }
            ip++;
            auto para_height = p_bottom - draw_pos.y-winpos.y;
            draw_pos.x = hmargin;
            draw_pos.y += para_height;
            ppara->set_size(para_width, para_height);
            ppara->draw();
          }
          ImGuiContext& g = *GImGui;
          ImGuiWindow* cur_window = ImGui::GetCurrentWindow();
          ImGuiWindow* front_window = g.Windows.back();
          ImRect wrect( cur_window->Pos, cur_window->Pos + cur_window->Size );
          function<void(base_ui_component*, base_ui_component*)> swap_ui =
              [&](base_ui_component* pui1,base_ui_component* pui2)
          {
            auto pparent1 = pui1->get_parent();
            auto pparent2 = pui2->get_parent();
            if (!pparent1 || !pparent2) {
              return;
            }
            if (pparent1 !=pparent2) {
              swap_ui(pparent1, pparent2);
            } else {
              int isiz = pparent1->get_child_count();
              int i1, i2;
              i1 = i2 = -1;
              for (int it = 0; it < isiz; it++) {
                auto pchd = pparent1->get_child(it);
                if (pchd == pui1) {
                  i1 = it;
                }
                if (pchd == pui2) {
                  i2 = it;
                }
                if (i1 != -1 && i2 != -1) {
                  break;
                }
              }
              pparent1->switch_chd(i1, i2);
            }
               
          };
          if( cur_window == front_window && wrect.Contains( ImGui::GetIO().MousePos ) )
          {
               static base_ui_component* psel_ui = nullptr;
               ImGuiIO& io = ImGui::GetIO();

               if( ImGui::IsMouseClicked( 0 ) )
               {
                    psel_ui = get_hit_ui_object(io.MousePos.x, io.MousePos.y );
               }
               if( psel_ui&&ImGui::IsMouseReleased( 0 ) )
               {
                    base_ui_component* phit = get_hit_ui_object( io.MousePos.x, io.MousePos.y );
                    
                    if (phit!=psel_ui&&typeid(*phit)!=typeid(psel_ui))
                    {
                         swap_ui( psel_ui, phit );
                         psel_ui = nullptr;
                         calculate_order();
                         if( _cur_order.is_orignal_order())
                         {
                              //printf( "msg_host consume%d milli seconds\n", delta );
                              _gaming = false;
                              _finish_start = steady_clock::now();
                              if (consume_seconds < sec_of_superb_score)
                              {
                                   game_finish_state = L"You did a unbelievable job!";
                                   game_score = en_superb;
                              }
                              else if(consume_seconds < sec_of_mid_score)
                              {
                                   game_finish_state = L"You did a excellent job!";
                                   game_score =en_mid;
                              }
                              else 
                              {
                                   game_finish_state = L"You did a good job!";
                                   game_score =en_pass;
                              }
                         } else {
                             game_state = "";
                             if (_cur_order.paragraphs_restored) {
                                 game_state += "main order of the essay is restored! ";
                             }
                             for(int ii=0;ii<_cur_order.paragraphs_order.size();ii++){
                                 if (_cur_order.paragraphs_order[ii]) {
                                     char numstr[20];
                                     itoa(ii,numstr,10);
                                     game_state += "paragraph";
                                     game_state += numstr;
                                     game_state += " is restored!";

                                }
                             }
                         } 
                    }
               }
               
               if( psel_ui&&ImGui::IsMouseDragging( 0 ) )
               {
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->PushClipRectFullScreen();
                    draw_list->AddLine( io.MouseClickedPos[ 0 ], io.MousePos, ImGui::GetColorU32( ImGuiCol_Button ), 4.0f );
                    draw_list->PopClipRect();
               }
               
          }
     }

     void ft_essay::shuffle()
     {
          _gaming = true;
          game_score = en_fail;
          game_state = "";
          _play_start = steady_clock::now();
          consume_seconds = 0;
          srand( unsigned( time( 0 ) ) );
          for (auto&chd : _vchilds) {
            ft_paragraph* ppara = static_cast<ft_paragraph*>(chd);
            ppara->shuffle();
          }
          random_shuffle( _vchilds.begin(), _vchilds.end() );
          _cur_order.paragraphs_restored = false;
          _cur_order.paragraphs_order.assign(_vchilds.size(), false);
     }

     void ft_essay::calculate_order()
     {
          int ix = 0;
          int oid = 0;
          for (auto& itc:_vchilds)
          {
            ft_paragraph* pst = static_cast<ft_paragraph*>(itc);
            if (pst->orignal_order()) {
                _cur_order.paragraphs_order[ix] = true;
            }
            if (pst->load_idx == ix)
            {
                oid++;
            }
            ix++;
          }
          _cur_order.paragraphs_restored = ix == oid;
     }

}