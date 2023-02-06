#pragma once
#include "ft_base.h"
namespace auto_future
{
     class ft_paragraph :
          public ft_base
     {
         af_vec4 _bdclr;
         float _weight=2.f;
     public:
        int load_idx = 0;
        bool orignal_order();
        ft_paragraph();
        ft_paragraph& set_border_color(float r, float g, float b, float a)
        {
            _bdclr.x = r;
            _bdclr.y = g;
            _bdclr.z = b;
            _bdclr.w = a;
            return *this;
        }
        ft_paragraph& set_weight(float weight) {
          _weight = weight;
          return *this;
        }
        void shuffle();      
        void draw();
     };
}
