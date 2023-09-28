#include "tar_approaching.h"
using namespace auto_future;

void set_tar_approaching(ft_trans* ptar, ft_trans* ptar_approaching,ft_light_scene* pproj){
	const float PI = 3.1415926545f;
	const float u_s = 1.f / 180.f;
	auto get_obj_rect = [](ft_trans* pobj, af_rect& rt) {
		auto sz_x = pobj->get_scale_x();
		auto sz_y = pobj->get_scale_y();
		auto left = pobj->get_translation_x()-sz_x*0.5f;
		auto top = sz_y;
		auto right = left + sz_x;
		auto bottom = 0.f;
		rt= { left,top,right,bottom };
	};
	af_rect rt_tar;
	get_obj_rect(ptar, rt_tar);
	auto tar_z_dis = ptar->get_translation_z();
	auto peye_pos = pproj->get_view_pos();

	auto pcenter_pos = pproj->get_center_of_prj();
	auto eye_obj_z = tar_z_dis - peye_pos->z;
	float proj_w=0.f, proj_h=0.f; 
	pproj->get_size(proj_w, proj_h);
	auto fovy = pproj->get_fovy();
	auto fovy_r = fovy * PI * u_s;
	auto eye_proj_z = pcenter_pos->z - peye_pos->z;
	auto z_span = eye_obj_z - eye_proj_z;
	if (z_span < 0.f) {
		return;
	}
	auto half_proj_h = eye_proj_z * tan(fovy_r * 0.5f);
	auto proj_top = pcenter_pos->y + half_proj_h;
	auto proj_bottom = pcenter_pos->y - half_proj_h;
	auto half_proj_w = half_proj_h * proj_w / proj_h;
	auto proj_left = pcenter_pos->x - half_proj_w;
	auto proj_right = pcenter_pos->x + half_proj_w;

	auto get_proj_y = [&](float y, float e_o_z)->float {
		float proj_y =peye_pos->y - (peye_pos->y - y)* e_o_z / eye_proj_z;
		return proj_y;
	};
	auto proj_top_at_ez = get_proj_y(proj_top, eye_obj_z);
	auto proj_bottom_at_ez= get_proj_y(proj_bottom, eye_obj_z);
	auto get_proj_x = [&](float x, float e_o_z)->float {
		float proj_x =peye_pos->x- (peye_pos->x-x)* e_o_z / eye_proj_z;
		return proj_x;
	};
	auto proj_left_at_ez = get_proj_x(proj_left, eye_obj_z);
	auto proj_right_at_ez = get_proj_x(proj_right, eye_obj_z);

	auto max = [](float v0, float v1) {
		if (v0 > v1)
			return v0;
		else
			return v1; };
	auto min = [](float v0, float v1) {
		if (v0 > v1)
			return v1;
		else
			return v0; };

	auto left_i = max(rt_tar.left(), proj_left_at_ez);
	auto right_i = min(rt_tar.right(), proj_right_at_ez);
	auto top_i = min(rt_tar.top(), proj_top_at_ez);
	auto bottom_i = max(rt_tar.bottom(), proj_bottom_at_ez);
	auto w_i = right_i - left_i;
	auto h_i = top_i - bottom_i;
	if (w_i > 0 && h_i > 0) {//intersect
		ptar_approaching->set_visible(false);
		return;
	}
	ptar_approaching->set_visible(true);
	const float max_zspan = 30;
	if (z_span > max_zspan) {
		z_span = max_zspan;
	}
	float alpha = 1 - z_span / max_zspan;
	auto pchild = (ft_rectangle_3d*)ptar_approaching->get_child(0);
	pchild->set_alpha(alpha);
	const float s_in = 0.2f;//µãÁÁ±ßÔµµÄ±ÈÀý
	float in_u = s_in * (proj_top - proj_bottom);
	float proj_in_left = proj_left_at_ez + in_u;
	float proj_in_right = proj_right_at_ez - in_u;
	float proj_in_top = proj_top_at_ez - in_u;
	float proj_in_bottom = proj_bottom_at_ez + in_u;
	af_rect rt_ap;
	if (rt_tar.right() < proj_in_left) {
		rt_ap.right_bottom.x = proj_in_left;
	}
	else {
		rt_ap.right_bottom.x = rt_tar.right();
	}
	if (rt_tar.left() > proj_in_right) {
		rt_ap.left_top.x = proj_in_right;
	}	else {
		rt_ap.left_top.x = rt_tar.left();
	}

	if (rt_tar.bottom() > proj_in_top) {
		rt_ap.right_bottom.y = proj_in_top;
	}	else {
		rt_ap.right_bottom.y = rt_tar.bottom();
	}

	if (rt_tar.top() < proj_in_bottom) {
		rt_ap.left_top.y = proj_in_bottom;
	}	else {
		rt_ap.left_top.y = rt_tar.top();
	}

	ptar_approaching->set_scale_x(rt_ap.width());
	ptar_approaching->set_scale_y(rt_ap.height());
	ptar_approaching->set_translation_z(tar_z_dis);
	auto ap_center = rt_ap.center();
	ptar_approaching->set_translation_x(ap_center.x)
		.set_translation_y(ap_center.y);
}