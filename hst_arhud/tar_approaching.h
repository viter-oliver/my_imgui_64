#ifndef TAR_APPROACHING_H
#define TAR_APPROACHING_H
#include "afg.h"
struct af_rect
{
	af_vec2 left_top, right_bottom;
	float left() { return left_top.x; }
	float right() { return right_bottom.x; }
	float top() { return left_top.y; }
	float bottom() { return right_bottom.y; }
	float width() { return right_bottom.x - left_top.x; }
	float height() { return left_top.y - right_bottom.y; }
	float h_w_scale() { return height() / width(); }
	af_vec2 center() { return { (left_top.x + right_bottom.x) * 0.5f,(left_top.y + right_bottom.y) * 0.5f }; }
};
void set_tar_approaching(auto_future::ft_trans* ptar, auto_future::ft_trans* ptar_approaching, auto_future::ft_light_scene* pproj);
#endif