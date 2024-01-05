#pragma once
#include "ft_base.h"
#include "af_font_res_set.h"
#include <chrono>
using namespace chrono;
namespace auto_future
{
     struct essay_order {
        bool paragraphs_restored  { false };
        std::vector<bool> paragraphs_order;
        bool is_orignal_order() {
            if (paragraphs_restored) {
                for (auto porder : paragraphs_order) {
                    if (!porder) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }
     };
<<<<<<< HEAD
     enum score_state {
       en_pass,
       en_mid,
       en_superb,
       en_fail,
       en_score_cnt
     };
     const float scale_score[en_score_cnt] = { 0.6f,0.8f,1.f,0.0f };
     
     using game_state_trigger = function<void(score_state st)>;
=======
>>>>>>> parent of aa0dd62 (display game)
     class ft_essay :
          public ft_base
     {
          ps_font_unit _pfont_unit;
          steady_clock::time_point  _play_start;
          essay_order _cur_order;
          bool _gaming{ false };
     public:
          float hmargin = { 20.f }, vmargin = { 30.f }, line_spacing = {10.f};
          int consume_seconds = 0;
		  int font_size = 20;
          string game_state;
          ft_essay();
          ~ft_essay();
          void set_font( ps_font_unit& pfont )
          {
               _pfont_unit = pfont;
          }
          bool is_gaming() { return _gaming; }
          void load_content( wstring& str_content );
          void draw();
          void shuffle();
          void calculate_order();
     };
}
