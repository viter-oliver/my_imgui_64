//
// Created by v_f_z on 2022/6/30.
//
#include <stdio.h>
#include <chrono>
#include "register_msg_host.h"
#include "adas_can_def.h"
#include "af_bind.h"
#include "af_state_manager.h"
#include "navi_turn.h"
#include "afg.h"
#include "debug_var_set.h"
#include "enum_txt_res_name.h"
#include "tar_approaching.h"
using namespace msg_utility;
using namespace auto_future;
using namespace std;
using namespace chrono;
static bool debug_on = false;
#define vg_print(...) \
  if (debug_on && 0 == debug_cnt) {                                            \
    printf("$$$%s:%d ", __func__, __LINE__);                                   \
    printf(__VA_ARGS__);                                                       \
  }
int debug_cnt = 0;
int debug_cirlce = 15;
float angle_to_s = 0.0174533f;
// XXX hud calibration data
namespace hud_cali {
/**
   |Z
y\ |
  \|____X

adas:
   |z
x\ |
  \|____y

opengl:
   |y
z\ |
  \|____X

eye positon   x=-0.37 y=1.18  z=-2.37

center of pj  x=-0.34 y=1.00 z=2.47 bottom=0.901 top=1.147 left=-1.31  right=1.05

fov=3.14
                                1.36
                                1.15
                                0.98
center of pj     x=-0.38      y=1.03     z=2.03
                                0.60
                                0.39
                                0.16

*/
const int level_cnt = 7;
int level{10};
af_vec3 center_pos[level_cnt] = {
  {-0.335f,0.99f,7.42f},
  {-0.335f,0.99f,7.42f},
  {-0.335f,0.99f,7.42f},
  {-0.335f,0.99f,7.42f},
  {-0.335f,0.99f,7.42f},
  {-0.335f,0.99f,7.42f},
  {-0.335f,0.99f,7.42f},
};
af_vec3 &cur_center_pos() {
    return center_pos[level];
}
float _fov{3.14f};
}
// #define HUD_DEBUG
namespace navi{
float speed_m_s = 0.f;
}
// UPDATE:namespace hud
namespace hud {
ft_light_scene *pprojector;
ft_trans *pleft_lane_trans;
ft_trans *pright_lane_trans;
ft_trans *pno_lane_change_trans;
ft_3_times_curve_3d *pleft_lane;
ft_3_times_curve_3d *pright_lane;
ft_3_time_wall_3d *pno_lane_change_wall;
ft_trans *pmain_tar;
ft_trans *ptar[4];
ft_trans *pdes;
ft_trans *pdesx[4];
ft_trans* ptar_approaching;
float ptar_z_v=0;
float pdes_x_v=0;
float pdes_z_v=0;
bool capture_video = false;
bool be_fcw = false;
float view_distance = 0.f;
class smoothor {
    static const int uvalue_len = 10;
    float ut_value[uvalue_len];
    int ut_id = {0};
    int buff_cnt = 0;
    float delta_max=0.f;
public:
    enum en_ag { en_linear_3_p, en_linear_5_p, en_linear_5_3, en_linear_cnt };
    void clear() { memset(ut_value, 0, sizeof ut_value); }
    smoothor() { clear(); }
    smoothor(float dm):delta_max(dm){clear();}

private:
    int ag_id = {en_linear_3_p};
    int ucnt() {
        switch (ag_id) {
            case smoothor::en_linear_3_p:
                return 3;
            case smoothor::en_linear_5_p:
                return 5;
            case smoothor::en_linear_5_3:
                return 5;
            default:
                return 3;
        }
    }

public:
    void set_arg(int aid) {
        aid %= en_linear_cnt;
        ag_id = aid;
        clear();
    }
    float smooth_value(float nf) {
        auto mvcnt = ucnt() - 1;
        for (int ix = 0; ix < mvcnt; ++ix) {
            ut_value[ix] = ut_value[ix + 1];
        }

        switch (ag_id) {
            case smoothor::en_linear_3_p:
            {
                auto span=nf-ut_value[1];
                if(span>delta_max||span<-delta_max){
                    ut_value[1]=ut_value[2]=nf;
                    return nf;
                } else {
                   float ut[3] = {ut_value[0], ut_value[1], nf};
                    ut_value[0] = (5 * ut[0] + 2 * ut[1] - ut[2]) / 6;
                    ut_value[1] = (ut[0] + ut[1] + ut[2]) / 3;
                    ut_value[2] = (2 * ut[1] - ut[0] + 5 * ut[2]) / 6;
                    return ut_value[2];
                }
                
            } break;
            case smoothor::en_linear_5_p:
            {
                float ut[5] = {ut_value[0], ut_value[1], ut_value[2], ut_value[3], nf};
                ut_value[0] = (3 * ut[0] + 2 * ut[1] + ut[2] - ut[4]) / 5;
                ut_value[1] = (4 * ut[0] + 3 * ut[1] + 2 * ut[2] + ut[3]) / 10;
                ut_value[2] = (ut[0] + ut[1] + ut[2] + ut[3] + ut[4]) / 5;
                ut_value[3] = (ut[1] + 2 * ut[2] + 3 * ut[3] + 4 * ut[4]) / 10;
                ut_value[4] = (ut[2] - ut[0] + 2 * ut[3] + 3 * ut[4]) / 5;
                return ut_value[4];
            } break;
            case smoothor::en_linear_5_3:
            {
                float ut[5] = {ut_value[0], ut_value[1], ut_value[2], ut_value[3], nf};
                ut_value[0] = (69 * ut[0] + 4 * (ut[1] + ut[3]) - 6 * ut[3] - ut[5]) / 70;
                ut_value[1] =
                    (2 * (ut[0] + ut[4]) + 27 * ut[1] + 12 * ut[2] - 8 * ut[3]) / 35;
                ut_value[2] =
                    (12 * (ut[1] + ut[3]) - 3 * (ut[0] + ut[4]) + 17 * ut[2]) / 35;
                ut_value[3] =
                    (2 * (ut[0] + ut[4]) - 8 * ut[1] + 12 * ut[2] + 27 * ut[3]) / 35;
                ut_value[4] = (4 * (ut[1] + ut[3]) - ut[0] + 6 * ut[2] + 69 * ut[4]) / 70;
                return ut_value[4];
            } break;
            default:
                break;
        }
        return 0.f;
    }
};
smoothor slc0(1.f), slc1, slc2, slc3, src0(1.f), src1, src2, src3;
#define SMOOTH_GET(m, v) hud::m = hud::s##m.smooth_value(v)
class dis_compensator{
steady_clock::time_point tp_pt;
const int innate_delay_milli=50;
public:
    dis_compensator(){
        tp_pt=steady_clock::now();
    }
    float dis_span(float velocity){
        auto nw_tm = steady_clock::now();
        int delta_milli = chrono::duration_cast<chrono::duration<int, std::milli>>(nw_tm - tp_pt).count();
        tp_pt=nw_tm;
        float delta_s=(delta_milli+innate_delay_milli)*0.001f;
        float offset=delta_s*velocity;
        return offset;
    }
};
void left_lane_clear() {
    slc0.clear();
    slc1.clear();
    slc2.clear();
    slc3.clear();
}
void right_lane_clear() {
    src0.clear();
    src1.clear();
    src2.clear();
    src3.clear();
}
void hide_all_objects() {
    //vg_print("hide all\n");
    pmain_tar->set_visible(false);
    pdes->set_visible(false);
    for (int id = 0; id < 4; ++id) {
        ptar[id]->set_visible(false);
        pdesx[id]->set_visible(false);
    }
}
void set_target_color(int tid, float distance) {
    const int segment_cnt = 3;
    const struct {
        af_vec3 mcol;
        float dis;
    } mcolor_d[segment_cnt] = {
      {1.0,0,20},//<20
      {1,1,0,50},//<50
      {0,1,0,10000},
    };
    for (int ix = 0; ix < segment_cnt; ix++) {
        if (distance < mcolor_d[ix].dis) {
            if (tid < 0) {
                auto prt = static_cast<ft_rectangle_3d *>(pmain_tar->get_child(0));
                prt->set_model_col(mcolor_d[ix].mcol);
            } else {
                auto prt = static_cast<ft_rectangle_3d *>(ptar[tid]->get_child(0));
                prt->set_model_col(mcolor_d[ix].mcol);
            }
            return;
        }
    }

}
void calculate_view_distance() {
    auto l1 = pprojector->get_center_of_prj()->z - pprojector->get_view_pos()->z;
    auto half_height = tan(pprojector->get_fovy() * angle_to_s) * l1 * 0.5f;
    auto h2 = pprojector->get_center_of_prj()->y + half_height;
    auto h1 = pprojector->get_view_pos()->y;
    view_distance = l1 * h2 / (h1 - h2);
}
bool left_lane_valid = false, right_lane_valid = false;
enum {
    free_lane_change,
    no_left_lane_change,
    no_right_lane_change,
} lane_change_status;
float lc0 = 0.f, lc1 = 0.f, lc2 = 0.f, lc3 = 0.f;
float rc0 = 0.f, rc1 = 0.f, rc2 = 0.f, rc3 = 0.f;

float adas_x = 0.f;
float adas_y = 0.f;
float adas_z = 0.f;
void set_fovy(float fovy) { pprojector->set_fovy(fovy); }
void set_clip(float ffar, float fnear) {
    pprojector->set_far(ffar);
    pprojector->set_near(fnear);
}
void set_far(ft_trans* pp,float dis){
    ft_rectangle_3d* pc=(ft_rectangle_3d* )pp->get_child(0);
    pc->set_far(dis);
}
void set_objs_far(float dis){
     set_far(pmain_tar,dis);
     set_far(pdes,dis);
     for(int i=0;i<4;++i){
        set_far(ptar[i],dis);
        set_far(pdesx[i],dis);
     }
}
bool valid_left_lane=false,valid_right_lane=false;
void set_objs_rotation_x(float rotation_x){
    hud::pdes->set_rotation_x(rotation_x);
    hud::pdesx[0]->set_rotation_x(rotation_x);
    hud::pdesx[1]->set_rotation_x(rotation_x);
    hud::pdesx[2]->set_rotation_x(rotation_x);
    hud::pdesx[3]->set_rotation_x(rotation_x);
    hud::pmain_tar->set_rotation_x(rotation_x);
    hud::ptar[0]->set_rotation_x(rotation_x);
    hud::ptar[1]->set_rotation_x(rotation_x);
    hud::ptar[2]->set_rotation_x(rotation_x);
    hud::ptar[3]->set_rotation_x(rotation_x);
}
void set_objs_rotation_y(float rotation_x) {
    hud::pdes->set_rotation_y(rotation_x);
    hud::pdesx[0]->set_rotation_y(rotation_x);
    hud::pdesx[1]->set_rotation_y(rotation_x);
    hud::pdesx[2]->set_rotation_y(rotation_x);
    hud::pdesx[3]->set_rotation_y(rotation_x);
    hud::pmain_tar->set_rotation_y(rotation_x);
    hud::ptar[0]->set_rotation_y(rotation_x);
    hud::ptar[1]->set_rotation_y(rotation_x);
    hud::ptar[2]->set_rotation_y(rotation_x);
    hud::ptar[3]->set_rotation_y(rotation_x);
}
void set_lane_rotation_y(float rotation_x) {
    hud::pleft_lane_trans->set_rotation_y(rotation_x);
    hud::pright_lane_trans->set_rotation_y(rotation_x);
}
void set_lane_rotation_x(float rotation_x) {
    hud::pleft_lane_trans->set_rotation_x(rotation_x);
    hud::pright_lane_trans->set_rotation_x(rotation_x);
}
void init_controls() {
    pprojector = (ft_light_scene *)get_aliase_ui_control("show_hud");
    /*auto& center_pos=*(pprojector->get_center_of_prj());
    center_pos=hud_cali::cur_center_pos();*/
    pleft_lane_trans = (ft_trans *)get_aliase_ui_control("show_left_lane");;
    pright_lane_trans = (ft_trans *)get_aliase_ui_control("show_right_lane");
    pleft_lane = (ft_3_times_curve_3d *)pleft_lane_trans->get_child(0);
    pright_lane = (ft_3_times_curve_3d *)pright_lane_trans->get_child(0);
    pno_lane_change_trans = (ft_trans *)get_aliase_ui_control("show_lane_change");
    pno_lane_change_wall = (ft_3_time_wall_3d *)pno_lane_change_trans->get_child(0);
    pmain_tar = (ft_trans *)get_aliase_ui_control("show_main_tar");
    ptar[0] = (ft_trans *)get_aliase_ui_control("show_tar0");
    ptar[1] = (ft_trans *)get_aliase_ui_control("show_tar1");
    ptar[2] = (ft_trans *)get_aliase_ui_control("show_tar2");
    ptar[3] = (ft_trans *)get_aliase_ui_control("show_tar3");
    pdes = (ft_trans *)get_aliase_ui_control("show_pdes");
    pdesx[0] = (ft_trans *)get_aliase_ui_control("show_pdes0");
    pdesx[1] = (ft_trans *)get_aliase_ui_control("show_pdes1");
    pdesx[2] = (ft_trans *)get_aliase_ui_control("show_pdes2");
    pdesx[3] = (ft_trans *)get_aliase_ui_control("show_pdes3");
    ptar_approaching = (ft_trans*)get_aliase_ui_control("show_tar_approaching");
    set_objs_far(30);
    set_objs_rotation_x(0.1);
    pprojector->get_view_pos()->y=1.35;
    pprojector->get_center_of_prj()->y=0.99;
    pprojector->get_center_of_prj()->x=-0.335;
    calculate_view_distance();
    lane_change_status = free_lane_change;
}
void calcu_left_lane() {
    vg_print("lc0=%f,lc1=%f,lc2=%f,lc3=%f\n", lc0, lc1, lc2, lc3)
    pleft_lane->set_coeff(lc0, lc1, lc2, lc3);
    if(no_left_lane_change==lane_change_status)
        pno_lane_change_wall->set_coeff(lc0-0.1, lc1, lc2, lc3);
}
void set_left_lane_range(float fnear, float ffar) {
    pleft_lane->set_near(fnear).set_far(ffar);
    pno_lane_change_wall->set_near(fnear).set_far(ffar);
}
void set_right_lane_range(float fnear, float ffar) {
    pright_lane->set_near(fnear).set_far(ffar);
    pno_lane_change_wall->set_near(fnear).set_far(ffar);
}
void calcu_right_lane() {
    vg_print("rc0=%f,rc1=%f,rc2=%f,rc3=%f\n", rc0, rc1, rc2, rc3)
    pright_lane->set_coeff(rc0, rc1, rc2, rc3);
    if(no_right_lane_change == lane_change_status)
        pno_lane_change_wall->set_coeff(rc0+0.1, rc1, rc2, rc3);
}

bool warning_no_lane_change(){
    return pno_lane_change_trans->is_visible();
}
steady_clock::time_point tp_no_lane_change;
void cancel_warning_no_lane_change(){
    if(warning_no_lane_change()){
        auto tp_now = steady_clock::now();
        auto dur_mills = duration_cast<milliseconds>(tp_now - tp_no_lane_change);
        if(dur_mills.count()>3000){
            pno_lane_change_trans->set_visible(false);
            lane_change_status = free_lane_change;
        }
    }
}
void no_lane_change(float obj_z,float obj_x){//trig warning no lane change
    if(!warning_no_lane_change()
    &&navi::speed_m_s>5.55
    &&obj_x<20){
        auto& z=obj_z;
        auto valid_slope = [](float c0,float c1,float c2,float c3) {
            const float c0_limit = 1.4, c1_limit = 0.025f, c2_limit = 0.001, c3_limit = 0.00002f;
            return c0 > -c0_limit && c0 < c0_limit&&
                c1 > -c1_limit && c1 < c1_limit&&
                c2 > -c2_limit && c2 < c2_limit&&
                c3 > -c3_limit && c3 < c3_limit;
        };
        if(valid_slope(lc0,lc1,lc2,lc3) && left_lane_valid) {
            auto left_lane_x_at_z=lc0+lc1*z+lc2*z*z+lc3*z*z;
            auto obj_left_offset=left_lane_x_at_z-obj_x;
            if(obj_left_offset>1&&obj_left_offset<3){
                pno_lane_change_trans->set_visible(true);
                tp_no_lane_change=steady_clock::now();
                lane_change_status = no_left_lane_change;
            }
            
        } else if(valid_slope(rc0,rc1,rc2,rc3) && right_lane_valid) {
            auto right_lane_x_at_z=rc0+rc1*z+rc2*z*z+rc3*z*z;
            auto obj_right_offset=obj_x-right_lane_x_at_z;
            if(obj_right_offset>1&&obj_right_offset<3){
                pno_lane_change_trans->set_visible(true);
                tp_no_lane_change=steady_clock::now();
                lane_change_status = no_right_lane_change;
            }
        }
    }
}
} // namespace hud

//UPDATE namespace navi
namespace navi {
float next_cross_distance = 0.f;
float rotate_angle_max = 0.f;
hud::smoothor sm_steering_angle;
float cur_steering_angle = 0.f;
const u16 straight_max = 120;
const u16 straight_limit = 80;
u16 keep_straight_cnt = 0;
s16 angle_accum = 0;
const float steering_angle0 = 1.f;
float steering_angle_peak = steering_angle0;
float ori_x = 0.f;
ft_trans *pnavi_ani_curl;
ft_rotate_curl_curve_3d *pnavi_ani_curl_wall;
ft_trans *panvi_map_point;
ft_trans *proad_surface;
ft_3_times_curve_3d *proad_surface_texture;
float steering_angle_fix;
float valid_rotate_angle_max = 1.f;
float navi_path_rotate_angle = 0.f;
float sum_dis_span = 0.f;
float sum_flag=0.f;
void init_controls() {
    pnavi_ani_curl = (ft_trans *)get_aliase_ui_control("show_navi_ani_curl");
    pnavi_ani_curl_wall = (ft_rotate_curl_curve_3d *)pnavi_ani_curl->get_child(0);
    panvi_map_point = (ft_trans *)get_aliase_ui_control("show_map_point");
    proad_surface = (ft_trans *)get_aliase_ui_control("show_road_surface");
    proad_surface_texture = (ft_3_times_curve_3d *)proad_surface->get_child(0);
}
bool will_triger_ani() {
    return rotate_angle_max > 0.001;
}
bool be_triger_ani() {
    return pnavi_ani_curl->is_visible() || proad_surface->is_visible();
}
bool be_steering_control() {
    return keep_straight_cnt > straight_limit;
}
void cancel_navi_ani() {
    if (be_triger_ani()) {
        cancel_play_tran("navi_ani_curl");
        cancel_play_tran("road_surface");
        set_property_aliase_value_T("show_navi_ani_curl", false);
        set_property_aliase_value_T("show_road_surface", false);
    }
}
bool be_navi_ani_left() {
    return ori_x < -0.00001f;
}
void ani_triger_type(int navi_id) {
    if (navi_id == 65) navi_id = 9;
    if (navi_id < 1 || navi_id>34) return;
    rotate_angle_max = prerotate_angle(navi_id, ori_x);
    if (will_triger_ani()) {
        valid_rotate_angle_max = rotate_angle_max;
        navi_path_rotate_angle = rotate_angle_max;
    }
    if (debug_cnt == 0) {
        if (navi_id == 2) {
            printf("<--\n");
        } else if (navi_id == 3) {
            printf("-->\n");
        } else if (navi_id == 4) {
            printf("<\ \n");
            printf("  \ \n");
        } else if (navi_id == 5) {
            printf(" /> \n");
            printf("/\n");
        } else if (navi_id == 6) {
            printf("  / \n");
            printf("</\n");
        } else if (navi_id == 7) {
            printf("\ \n");
            printf(" \> \n");
        } else if (navi_id == 8) {
            printf("|\n");
            printf("<\n");
        } else if (navi_id == 19) {
            printf("|\n");
            printf(">\n");
        }
    }
    vg_print("navi_id=%d\n", navi_id);
        int navi_icon_id = navi_id + en_gid_0destination_png;
    set_property_txt_aliase_value("navi_turn", navi_icon_id);
}
void trig_ani() {
    //XXX will cancel triger
    if (keep_straight_cnt > 0) {
        keep_straight_cnt--;
        if (keep_straight_cnt == 0) {
            cancel_navi_ani();
            next_cross_distance = 135.f;
        } else if (keep_straight_cnt < straight_limit) {
            float alpha = (float)keep_straight_cnt / (float)straight_limit;
            float angle_fix = steering_angle_fix * alpha;
            if (angle_fix < 0.04) {
                keep_straight_cnt = 1;
            }
            pnavi_ani_curl_wall->set_rotate_angle(angle_fix).set_alpha(alpha);
            vg_print("angle fix: %f\n", angle_fix);
        }
    }

    static struct {
        float _speed_min;
        float _rm_dis;
    } navi_ani_triger_tb[5] =
    {
      {28.77f,360.f},
      {22.16f,280.f},
      {16.62,250.f},
      {8.1,200.f},
      {0.f,130.f},
    };
    const float valid_distance_min=15.f,valid_distance_max=28.f;
    if (!be_triger_ani()) {
        if (will_triger_ani()) {
            bool is_triggered = false;
            if (next_cross_distance <= hud::view_distance) {
                is_triggered = true;
            }
#if 0
            for (u8 id = 0; id < 5; ++id) {
                if (speed_m_s >= navi_ani_triger_tb[id]._speed_min) {
                    is_triggered = next_cross_distance <= navi_ani_triger_tb[id]._rm_dis;
                    break;
                }
            }
#endif
            if (is_triggered) {
                //XXX trig navi ani
                steering_angle_peak = steering_angle0;
                //set_property_aliase_value_T("show_navipath",true);
                float n_dis = 0.f;
                if (next_cross_distance > valid_distance_max) {
                    n_dis = valid_distance_max;
                } else if (next_cross_distance < valid_distance_min) {
                    n_dis = valid_distance_min;
                } else {
                    n_dis = next_cross_distance;
                }
                //XXX set cross type and max roatate angle
                pnavi_ani_curl_wall->set_ori_x(ori_x);
                pnavi_ani_curl_wall->set_rotate_angle(valid_rotate_angle_max)
                    .set_alpha(1.f);
                steering_angle_fix = valid_rotate_angle_max;
                pnavi_ani_curl->set_translation_z(n_dis);
                set_property_aliase_value_T("show_road_surface", true);
                //play_tran("road_surface", 0, 1, 0);
                play_tran_playlist("road_surface", 0,0);
            }
        }
    } else {
        if (next_cross_distance >= valid_distance_min 
        && next_cross_distance <=valid_distance_max) {
            pnavi_ani_curl->set_translation_z(next_cross_distance);
        }
    }
}
void vspeed_update(float vspeed) {
    speed_m_s = vspeed;
    static steady_clock::time_point tp_prev_vspeed_update;
    auto tp_now = steady_clock::now();
    auto dur_mills = duration_cast<milliseconds>(tp_now - tp_prev_vspeed_update);
    tp_prev_vspeed_update = tp_now;
    float dur_t_s = dur_mills.count() * 0.001f;
    float dis_span = speed_m_s * dur_t_s;
    /*printf("nx_dis=%f,dis_span=%f,v=%f,t=%f\n",
    next_cross_distance,dis_span,speed_m_s,dur_t_s);*/
    sum_dis_span += dis_span;
    return;
    if (next_cross_distance > dis_span) {
        next_cross_distance -= dis_span;
    } else {
        next_cross_distance = 0;
    }
}
void navi_distance_update(int distance) {
    vg_print("distance to next cross is %d m\n", distance);
        char str_dm[50] = {0};
    if (distance < 1000) {
        sprintf(str_dm, "%dm", distance);
    } else {
        int dis_k = distance / 1000;
        sprintf(str_dm, "%dkm", dis_k);
    }
    set_property_aliase_value("distance_remaining", str_dm);
    float fdistance = distance;
    if (0 == distance) {
        return;
    }
    auto real_dis_span = fdistance - next_cross_distance;
    if (real_dis_span > 30) {
        next_cross_distance = fdistance;
        if (!be_steering_control()) {
            cancel_navi_ani();
            keep_straight_cnt = 0;
        } else {
            keep_straight_cnt = straight_max;//maybe a new cross is coming
        }
        printf("$$$$$$$$$$$$$$$next dis\n");
    } else if (real_dis_span < 0) {
        next_cross_distance = fdistance;
    }
}
bool be_turning(s16 angle) {
    const s16 max_acc = 20, angle_limit = 150;
    if (angle <= angle_limit) {
        angle_accum = 0;
        sm_steering_angle.clear();
        return false;
    }
    static s16 prev_angle = 0;
    if (angle > prev_angle) {
        angle_accum++;
    }
    prev_angle = angle;
    return angle_accum<-max_acc || angle_accum>max_acc;
}
void find_peak(s16 angle) {
    if (angle > steering_angle_peak) {
        steering_angle_peak = angle;
    }
}
void steering(s16 angle) {
    float st_angle = angle * 0.1f;//-500,500
    cur_steering_angle = st_angle;
    const float max_angle = 1.57f;//  0.5235988f;
    const float st_scale = 0.002f;
    float rst_angle = st_angle * max_angle * st_scale;
    set_property_aliase_value("steering_angle", &rst_angle);
    vg_print("steering angel=%f ,%f\n", st_angle, rst_angle)
        //XXX:control rotate angle of navi animation
    if(angle<0&&be_navi_ani_left()
    ||angle>0&&!be_navi_ani_left()){
        return;
    }
        if (angle < 0) {
            angle = -angle;
        }

    if (be_turning(angle)
        && be_triger_ani()) {
        find_peak(angle);
        navi_path_rotate_angle = angle * valid_rotate_angle_max / steering_angle_peak;
        navi_path_rotate_angle = sm_steering_angle.smooth_value(navi_path_rotate_angle);
        keep_straight_cnt = straight_max;
        vg_print("navi rotate angle=%f,pk=%f,abs=%d\n", navi_path_rotate_angle, steering_angle_peak, angle);
        pnavi_ani_curl_wall->set_rotate_angle(navi_path_rotate_angle);
        if (navi_path_rotate_angle < 0.001) {
            vg_print("navi rotate angle is 0\n");
        }
        steering_angle_fix = navi_path_rotate_angle;
    }
}
void trig_map_point(int distance) {
    if (distance < hud::view_distance) {// show map point when close view_distance
        panvi_map_point->set_translation_z(distance);
        if (!panvi_map_point->is_visible()) {
            set_property_aliase_value_T("show_map_point", true);
            play_tran("map_point_rotation", 0, 1, 1000);
        }
    } else {
        set_property_aliase_value_T("show_map_point", false);
    }
}
}
// TODO register debug interface
void reg_debug() {
    fifo_debuger::reg_command_handle([](char *pcmd_buff) {
        string str_cmd(pcmd_buff);
        auto eq_pos = str_cmd.find_last_of('=');
        if (eq_pos != -1) {
            string alias_name = str_cmd.substr(0, eq_pos);
            string str_value = str_cmd.substr(eq_pos + 1);
            auto pt_pos = str_value.find('.');
            if (pt_pos != -1) {
                float fvalue = atof(str_value.c_str());
                printf("f alias:%s =%f\n", alias_name.c_str(), fvalue);
                set_property_aliase_value_T(alias_name, fvalue);
            } else {
                int ivalue = atoi(str_value.c_str());
                printf("i alias:%s =%d\n", alias_name.c_str(), ivalue);
                set_property_aliase_value_T(alias_name, ivalue);
            }
        }
        return true;
        });
    fifo_debuger::attach_var_handle("debug_on", [](char *pcmd_buff) -> bool {
        debug_on = atoi(pcmd_buff);
        return true;
        });

    fifo_debuger::attach_var_handle("debug_circle", [](char *pcmd_buff) -> bool {
        debug_cirlce = atoi(pcmd_buff);
        return true;
        });
    fifo_debuger::attach_var_handle("capture_video", [](char *pcmd_buff) -> bool {
        const af_vec3 col_r = {1,0,0};
        const af_vec3 col_g = {0,1,0};
        hud::capture_video = atoi(pcmd_buff);
        if (hud::capture_video) {
            hud::pleft_lane_trans->set_visible(true);
            hud::pright_lane_trans->set_visible(true);
            hud::pleft_lane->set_lane_color(col_g);
            hud::pright_lane->set_lane_color(col_g);
            //TODO:set view pos
            //*hud::pprojector->get_view_pos() = {-0.37f,1.30f,-2.37f};
        } else {
            hud::pleft_lane_trans->set_visible(false);
            hud::pright_lane_trans->set_visible(false);
            hud::pleft_lane->set_lane_color(col_r);
            hud::pright_lane->set_lane_color(col_r);
            //*hud::pprojector->get_view_pos() = {-0.37f,1.30f,-2.37f};
        }
        return true;
        });
    fifo_debuger::attach_var_handle("fovy", [](char *pcmd_buff) -> bool {
        float fovy = atof(pcmd_buff);
        hud::pprojector->set_fovy(fovy);
        return true;
        });

    fifo_debuger::attach_var_handle("prj_pos", [](char *pcmd_buff) -> bool {
        auto &prj_center = *hud::pprojector->get_center_of_prj();
        auto &prj_view = *hud::pprojector->get_view_pos();
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("view_x=%f\nview_y=%f\nview_z=%f\n", prj_view.x, prj_view.y, prj_view.z);
        printf("center_x=%f\ncenter_y=%f\ncenter_z=%f\n", prj_center.x, prj_center.y, prj_center.z);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        return true;
        });
    fifo_debuger::attach_var_handle("sum_dis_span", [](char *pcmd_buff) -> bool {
        int sum_flag = atoi(pcmd_buff);
        float dis_span = navi::sum_dis_span;
        static steady_clock::time_point dis_span_tp;
        if(sum_flag > 0){
            dis_span_tp = steady_clock::now();
            navi::sum_dis_span = 0.f;
        }else{
            auto tp_now = steady_clock::now();
            auto dur_mills = duration_cast<milliseconds>(tp_now - dis_span_tp);
            float dur_t_s = dur_mills.count() * 0.001f;
            printf("sum dis span is:%f", dis_span);
            printf("sum time is:%f",dur_t_s);
        }
        return true;
        });
    fifo_debuger::attach_var_handle("view_x", [](char *pcmd_buff) -> bool {
        float view_x = atof(pcmd_buff);
        hud::pprojector->get_view_pos()->x = view_x;
        return true;
        });
    fifo_debuger::attach_var_handle("view_y", [](char *pcmd_buff) -> bool {
        float view_y = atof(pcmd_buff);
        hud::pprojector->get_view_pos()->y = view_y;
        return true;
        });

    fifo_debuger::attach_var_handle("view_z", [](char *pcmd_buff) -> bool {
        float view_z = atof(pcmd_buff);
        hud::pprojector->get_view_pos()->z = view_z;
        return true;
        });
    fifo_debuger::attach_var_handle("center_x", [](char *pcmd_buff) -> bool {
        float center_x = atof(pcmd_buff);
        hud::pprojector->get_center_of_prj()->x = center_x;
        return true;
        });
    fifo_debuger::attach_var_handle("center_y", [](char *pcmd_buff) -> bool {
        float center_y = atof(pcmd_buff);
        hud::pprojector->get_center_of_prj()->y = center_y;
        return true;
        });
    fifo_debuger::attach_var_handle("center_z", [](char *pcmd_buff) -> bool {
        float center_z = atof(pcmd_buff);
        hud::pprojector->get_center_of_prj()->z = center_z;
        return true;
        });
    fifo_debuger::attach_var_handle("adas_x", [](char *pcmd_buff) -> bool {
        float adas_x = atof(pcmd_buff);
        hud::pleft_lane_trans->set_translation_x(adas_x);
        hud::pright_lane_trans->set_translation_x(adas_x);
        hud::adas_x = adas_x;
        return true;
        });
    fifo_debuger::attach_var_handle("adas_y", [](char *pcmd_buff) -> bool {
        float adas_y = atof(pcmd_buff);
        hud::pleft_lane_trans->set_translation_y(adas_y);

        hud::pright_lane_trans->set_translation_y(adas_y);
        hud::adas_y = adas_y;
        return true;
        });
    fifo_debuger::attach_var_handle("adas_z", [](char *pcmd_buff) -> bool {
        float adas_z = atof(pcmd_buff);
        hud::pleft_lane_trans->set_translation_z(adas_z);
        hud::pright_lane_trans->set_translation_z(adas_z);
        hud::pno_lane_change_wall->set_base_z(adas_z);
        hud::adas_z = adas_z;
        return true;
        });
    fifo_debuger::attach_var_handle("pdes_y", [](char *pcmd_buff) -> bool {
        float pdes_y = atof(pcmd_buff);
        hud::pdes->set_rotation_y(pdes_y);
        hud::pdesx[0]->set_rotation_y(pdes_y);
        hud::pdesx[1]->set_rotation_y(pdes_y);
        hud::pdesx[2]->set_rotation_y(pdes_y);
        hud::pdesx[3]->set_rotation_y(pdes_y);
        return true;
        });

    fifo_debuger::attach_var_handle("tar_rotation_x", [](char *pcmd_buff) -> bool {
        float rotation_x = atof(pcmd_buff);
        hud::set_objs_rotation_x(rotation_x);
        return true;
        });
    fifo_debuger::attach_var_handle("tar_rotation_y", [](char *pcmd_buff) -> bool {
        float rotation_x = atof(pcmd_buff);
        hud::set_objs_rotation_y(rotation_x);
        return true;
        });
    fifo_debuger::attach_var_handle("lane_rotation_x", [](char *pcmd_buff) -> bool {
        float rotation_x = atof(pcmd_buff);
        hud::set_lane_rotation_x(rotation_x);
        return true;
        });
    fifo_debuger::attach_var_handle("lane_rotation_y", [](char *pcmd_buff) -> bool {
        float rotation_x = atof(pcmd_buff);
        hud::set_lane_rotation_y(rotation_x);
        return true;
        });

}

void register_msg_host(msg_utility::msg_host &mh) {
    // UPDATE register adas interface
    {
#define POS_FACTOR 0.00390625
#define CUR_FACTOR 0.000000976563
#define CUR_OFFSET 0.031999023438
#define CUR_DE_FACTOR 0.00000000372529
#define CUR_DE_OFFSET 0.00012206658721
#define VEHICLE_V_FACTOR 0.0625
        mh.register_batch_cmd_handle([](u8 *pbuff, u32 len) -> bool {
          u16 *pcmd = (u16 *)pbuff;
          if (num_in_range(*pcmd, Vehicle_frame_b0, Vehicle_frame_b7)) {
            auto frame_id = *pcmd - Vehicle_frame_b0;
            if (frame_id > 1)
              return false;
            auto base_id = frame_id * 2;
            struct GNU_DEF Vehicle_frame_bx {
              u8 Addition_Vehicle_Number : 2;
              u8 Vehicle_ID : 6;
              u16 Addition_Vehicle_A_PosX : 12; // factor 0.0625
              s16 Addition_Vehicle_A_PosY : 10; // factor 0.0625
              u16 Addition_Vehicle_A_Type : 3;
              u16 Vehicle_ID_2 : 6;
              u16 Addition_Vehicle_B_PosX : 12; // factor 0.0625
              s16 Addition_Vehicle_B_PosY : 10; // factor 0.0625
              u16 Addition_Vehicle_B_Type : 3;  // no
                                                // vehicle,car,bus,truck,special
                                                // car
            };
            pbuff += 2;
            Vehicle_frame_bx *pcan = (Vehicle_frame_bx *)pbuff;
            auto add_v_num = pcan->Addition_Vehicle_Number;
            float Addition_Vehicle_A_PosX =
                pcan->Addition_Vehicle_A_PosX * 0.0625f + hud::adas_z;
            float Addition_Vehicle_A_PosY =
                pcan->Addition_Vehicle_A_PosY * 0.0625f;
            float Addition_Vehicle_B_PosX =
                pcan->Addition_Vehicle_B_PosX * 0.0625f + hud::adas_z;
            float Addition_Vehicle_B_PosY =
                pcan->Addition_Vehicle_B_PosY * 0.0625f;
            hud::no_lane_change(Addition_Vehicle_A_PosX,
                                Addition_Vehicle_A_PosY);
            hud::no_lane_change(Addition_Vehicle_B_PosX,
                                Addition_Vehicle_B_PosY);
#if 0
                vg_print("car id:%d posx=%f,posy=%f \n", pcan->Vehicle_ID,
                    Addition_Vehicle_A_PosX, Addition_Vehicle_A_PosY)
                    vg_print("car2 id:%d posx=%f,posy=%f frame_id=%d vnm=%d\n", pcan->Vehicle_ID_2, Addition_Vehicle_B_PosX, Addition_Vehicle_B_PosY,
                        frame_id, add_v_num)
#endif
            auto catch_a = pcan->Addition_Vehicle_A_Type != 0;
            auto catch_b = pcan->Addition_Vehicle_B_Type != 0;

            hud::ptar[base_id]
                ->set_translation_z(Addition_Vehicle_A_PosX)
                .set_translation_x(Addition_Vehicle_A_PosY)
                .set_visible(catch_a);
            hud::set_target_color(base_id, Addition_Vehicle_A_PosX);
            hud::ptar[base_id + 1]
                ->set_translation_z(Addition_Vehicle_B_PosX)
                .set_translation_x(Addition_Vehicle_B_PosY)
                .set_visible(catch_b);
            hud::set_target_color(base_id + 1, Addition_Vehicle_B_PosX);
                if ((Addition_Vehicle_A_PosX > 1)
                    && (Addition_Vehicle_A_PosX < 50.f)) {
                   vg_print("car[%d] posx=%f, posy=%f\n", base_id, Addition_Vehicle_A_PosX, Addition_Vehicle_A_PosY);
                }
                if ((Addition_Vehicle_B_PosX > 1)
                    && (Addition_Vehicle_B_PosX < 50.f)) {
                   vg_print("car[%d] posx=%f, posy=%f\n", base_id + 1, Addition_Vehicle_B_PosX, Addition_Vehicle_B_PosX);
                }

          } else if (num_in_range(*pcmd, Pedestrian_frame_bb,
                                  Pedestrian_frame_bd)) {
            auto frame_id = *pcmd - Pedestrian_frame_bb;
            if (frame_id > 1)
              return false;
            auto base_id = frame_id * 2;
            pbuff += 2;
            struct GNU_DEF St_Pedestrian_frame_bx {
              u8 Addition_Pedestrian_Number : 2;
              u8 Pedestrian_ID : 6;
              u16 Addition_Pedestrian_A_PosX : 12; // 0.0625 0
              u16 Addition_Pedestrian_A_PosY : 10; // 0.0625 -32
              u16 Target_Pedestrian_A_Type : 3;
              u8 Pedestrian_ID_2 : 6;
              u16 Addition_Pedestrian_B_PosX : 12; // 0.0625 0
              u16 Addition_Pedestrian_B_PosY : 10; // 0.0625 -32
              u16 Target_Pedestrian_B_Type : 3;
            };
            St_Pedestrian_frame_bx *pcan = (St_Pedestrian_frame_bx *)pbuff;
            float pdes_a_posx =
                pcan->Addition_Pedestrian_A_PosX * 0.0625 + hud::adas_z;
            float pdes_a_posy = pcan->Addition_Pedestrian_A_PosY * 0.0625;
            auto catch_a = pcan->Pedestrian_ID != 0;
            float pdes_b_posx =
                pcan->Addition_Pedestrian_B_PosX * 0.0625 + hud::adas_z;
            float pdes_b_posy = pcan->Addition_Pedestrian_B_PosY * 0.0625;
            auto catch_b = pcan->Pedestrian_ID_2 != 0;
                if ((pdes_a_posx > 1)
                    && (pdes_a_posx < 50.f)) {
                    vg_print("pdesx[%d] posx=%f, posy=%f\n", base_id, pdes_a_posx, pdes_a_posy);
                }
                if ((pdes_b_posx > 1)
                    && (pdes_b_posx < 50.f)) {
                    vg_print("pdesx[%d] posx=%f, posy=%f\n", base_id + 1, pdes_b_posx, pdes_b_posy);
                }
                hud::pdesx[base_id]
                   ->set_translation_z(pdes_a_posx)
                    .set_translation_x(pdes_a_posy)
                    .set_visible(catch_a);
                hud::pdesx[base_id + 1]
                   ->set_translation_z(pdes_b_posx)
                    .set_translation_x(pdes_b_posy)
                    .set_visible(catch_b);
          } else if (num_in_range(*pcmd, TSR_frame_B1, TSR_frame_B3)) {
            struct GNU_DEF st_TSR_frame_B {
              u8 Vision_only_Sign_Type;
              u32 reserved : 24;
              u32 CanMsgEnder : 1;
              u32 reserved2 : 7;
            };
          }
          return false;
        });
        mh.register_msg_handle(Vehicle_frame_a1, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_Vehicle_frame_a1 {
            u8 FCW : 1;
            u8 reserved_1 : 1;
            u8 Vehicle_ID : 6;
            u16 Target_Vehicle_PosX : 12; // factor:0.0625
            u16 reseved_2 : 4;
            u8 reseved_2_2 : 8;
            u16 Target_Vehicle_PosY : 10; // factor:0.0625 -31.9375
            u16 reseved_3 : 6;
            u16 Target_Vehicle_Type : 3;
            u16 reseved_4 : 13;
          };
          St_Vehicle_frame_a1 *pcan = (St_Vehicle_frame_a1 *)pbuff;
          float Target_Vehicle_PosX =
              pcan->Target_Vehicle_PosX * 0.0625f + hud::adas_z;
          {
            static hud::dis_compensator tar_z_dc;
            auto offset = tar_z_dc.dis_span(hud::ptar_z_v);
            Target_Vehicle_PosX += offset;
          }
          float Target_Vehicle_PosY = pcan->Target_Vehicle_PosY * 0.0625f;
          hud::no_lane_change(Target_Vehicle_PosX, Target_Vehicle_PosY);
          u16 *pcanid = (u16 *)pbuff;
          auto catch_car = pcan->Target_Vehicle_Type != 0;
          hud::pmain_tar->set_visible(catch_car);
          hud::ptar_approaching->set_visible(catch_car);
          hud::pmain_tar->set_translation_z(Target_Vehicle_PosX)
              .set_translation_x(Target_Vehicle_PosY);
          set_tar_approaching(hud::pmain_tar, hud::ptar_approaching, hud::pprojector);
          if (!hud::be_fcw)
            hud::set_target_color(-1, Target_Vehicle_PosX);

          return false;
        });
        mh.register_msg_handle(Vehicle_frame_a2, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_Vehicle_frame_a2 {
            u16 CAN_VIS_OBS_TTC_WITH_ACC : 10; // 0.01
            u16 reseved_1 : 6;
            s16 Target_Vehicle_VelX : 12; // 0.0625
            u16 reseved_2 : 4;
            u16 Target_Vehicle_AccelX : 10; // 0.03
            u16 reseved_3 : 6;
            u8 Target_Vehicle_Width;          // 0.05
            u8 Target_Vehicle_Confidence : 6; // 0.02
            u8 reseved_4 : 2;
          };
          St_Vehicle_frame_a2 *pcan = (St_Vehicle_frame_a2 *)pbuff;
          hud::ptar_z_v = pcan->Target_Vehicle_VelX * VEHICLE_V_FACTOR;
          float Target_Vehicle_Width = pcan->Target_Vehicle_Width * 0.05f;
          //  printf("main car width=%f\n",Target_Vehicle_Width);
          hud::pmain_tar->set_scale_x(Target_Vehicle_Width);
          return false;
        });
        mh.register_msg_handle(Vehicle_frame_c, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_Vehicle_frame_c {
            u32 frame_id;
            u8 speed;
          };
          St_Vehicle_frame_c *pcan = (St_Vehicle_frame_c *)pbuff;
          return false;
        });
        mh.register_msg_handle(Pedestrian_frame_a, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_Pedestrian_frame_a {
            u8 Target_Pedestrian_ID : 6;
            u8 reserved_1 : 2;
            u16 Target_Pedestrian_PosX : 12; // 0.0625
            s16 Target_Pedestrian_PosY : 10; // 0.0625 -32.0
            u16 ttc : 10;                    // 0.01
            s16 Target_Pedestrian_VelX : 12; // 0.0625 -127.93
            s16 Target_Pedestrian_VelY : 8;  // 0.0625  -8
            u16 Target_Pedestrian_Type : 3;
            u16 reserved_2 : 1;
          };
          St_Pedestrian_frame_a *pcan = (St_Pedestrian_frame_a *)pbuff;
          float pposx =
              pcan->Target_Pedestrian_PosX * VEHICLE_V_FACTOR + hud::adas_z;
          float pposy = pcan->Target_Pedestrian_PosY * VEHICLE_V_FACTOR;
          float pdes_z_v = pcan->Target_Pedestrian_VelX * VEHICLE_V_FACTOR;
          float pdes_x_v = pcan->Target_Pedestrian_VelY * VEHICLE_V_FACTOR;
          {
            static hud::dis_compensator pz, px;
            auto zoffset = pz.dis_span(pdes_z_v);
            pposx += zoffset;
            auto xoffset = px.dis_span(pdes_x_v);
            pposy += xoffset;
          }
            if ((pposx > 1)
                && (pposx < 50.f)) {
                vg_print("pedestrain posx=%f posy=%f\n", pposx, pposy);
            }
          hud::pdes->set_translation_z(pposx)
              .set_translation_x(pposy)
              .set_visible(true);
            return false;
        });
        mh.register_msg_handle(lane_frame_1_l, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_lane_frame_1_l {
            u8 Lane_Type : 4;
            u8 Quality : 2;
            u8 reserved : 2;         // factor 0.0625
            s16 Position;            // 0.00391 -128 128
            u16 Curvature;           // 0.000000976563 -0.032
            u16 CurvatureDerivative; // 0.00000000372529 -0.0001
            u8 WidthMarking;         // 0.01
          };

          St_lane_frame_1_l *pcan = (St_lane_frame_1_l *)pbuff;
          if (pcan->Quality <= adas_value::Low) {
            hud::left_lane_valid = false;
            hud::left_lane_clear();
            return false;
          }
          hud::left_lane_valid = true;
          float l_offset = pcan->Position * POS_FACTOR;
          {
            static hud::dis_compensator dc_lc0;
            auto yaw_angle = atan(hud::lc1);
            auto x_offset_speed = navi::speed_m_s * sin(yaw_angle);
            auto x_offset = dc_lc0.dis_span(x_offset_speed);
            l_offset += x_offset;
          }
          SMOOTH_GET(lc0, l_offset);
          SMOOTH_GET(lc2, pcan->Curvature * CUR_FACTOR - CUR_OFFSET);
          SMOOTH_GET(lc3,
                     pcan->CurvatureDerivative * CUR_DE_FACTOR - CUR_DE_OFFSET);
          hud::calcu_left_lane();
#if 0
      int delta = chrono::duration_cast<chrono::duration<int, std::milli>>(
                      nw_tm - tp_lc0)
                      .count();
      static int dcnt = 0;
      if (dcnt == 0) {
        vg_print("lc0 time span=%d millisec\n", delta)
      }
      dcnt++;
      dcnt %= 10;
      tp_lc0 = nw_tm;
#endif
          return false;
        });
        mh.register_msg_handle(lane_frame_1_h, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_lane_frame_1_h {
            u16 Heading_Angle;    // 0.0000108949 -0.357
            u8 ViewRangeStart;    // 0.5
            u8 ViewRangeEnd;      // 0.5
            u8 LaneCrossing : 1;  // 0-no crossing,1-crossing
            u8 LaneMarkColor : 3; // 0-white,1-yellow
            u8 resv : 4;
            u8 resv0;
          };
          if (!hud::left_lane_valid) {
            return false;
          }
          St_lane_frame_1_h *pcan = (St_lane_frame_1_h *)pbuff;
          float fn = pcan->ViewRangeStart * 0.5f;
          float ff = pcan->ViewRangeEnd * 0.5f;
          // vg_print("view near=%f far=%f\n",fn,ff)
          hud::set_left_lane_range(fn, ff);
          SMOOTH_GET(lc1, pcan->Heading_Angle * 0.0000108949 - 0.357);
          hud::calcu_left_lane();
          return false;
        });
        mh.register_msg_handle(lane_frame_2_l, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_lane_frame_2_l {
            u8 Lane_Type : 4;
            u8 Quality : 2;
            u8 reserved : 2;         // factor 0.0625
            s16 Position;            // 0.00390625 -128
            u16 Curvature;           // 0.000000976563 -0.032
            u16 CurvatureDerivative; // 0.00000000372529 -0.0001
            u8 WidthMarking;
          };
          St_lane_frame_2_l *pcan = (St_lane_frame_2_l *)pbuff;
          if (pcan->Quality <= adas_value::Low) {
            hud::right_lane_valid = false;
            hud::right_lane_clear();
            return false;
          }
          hud::right_lane_valid = true;
          float r_offset = pcan->Position * POS_FACTOR;
          {
            static hud::dis_compensator dc_rc0;
            auto yaw_angle = atan(hud::rc1);
            auto x_offset_speed = navi::speed_m_s * sin(yaw_angle);
            auto x_offset = dc_rc0.dis_span(x_offset_speed);
            r_offset += x_offset;
          }
          SMOOTH_GET(rc0, r_offset);
          SMOOTH_GET(rc2, pcan->Curvature * CUR_FACTOR - CUR_OFFSET);
          SMOOTH_GET(rc3,
                     pcan->CurvatureDerivative * CUR_DE_FACTOR - CUR_DE_OFFSET);
          hud::calcu_right_lane();
          return false;
        });
        mh.register_msg_handle(lane_frame_2_h, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_lane_frame_2_h {
            u16 Heading_Angle;    // 0.0000108949
            u8 ViewRangeStart;    // 0.5
            u8 ViewRangeEnd;      // 0.5
            u8 LaneCrossing : 1;  // 0-no crossing,1-crossing
            u8 LaneMarkColor : 3; // 0-white,1-yellow
          };
          if (!hud::right_lane_valid) {
            return false;
          }
          St_lane_frame_2_h *pcan = (St_lane_frame_2_h *)pbuff;
          float fn = pcan->ViewRangeStart * 0.5f;
          float ff = pcan->ViewRangeEnd * 0.5f;
          // vg_print("view near=%f far=%f\n",fn,ff)
          hud::set_right_lane_range(fn, ff);
          SMOOTH_GET(rc1, pcan->Heading_Angle * 0.0000108949 - 0.357);
          hud::calcu_right_lane();
          return false;
        });
        mh.register_msg_handle(TSR_frame_A, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_TSR_frame_A {
            u8 Vision_only_Sign_Type;
            u8 reserved[3];
            u8 Vision_only_Sign_Type2;
            u8 reserved2[3];
          };
          St_TSR_frame_A *pcan = (St_TSR_frame_A *)pbuff;
          const char *limit_speed[0xE] = {"10",  "20",  "30",  "40", "50",
                                          "60",  "70",  "80",  "90", "100",
                                          "110", "120", "130", "140"};
          const char *limit_speed2[0xF] = {"5",   "15",  "25",  "35",  "45",
                                           "55",  "65",  "75",  "85",  "95",
                                           "105", "115", "125", "135", "145"};
          if (pcan->Vision_only_Sign_Type >= 100) {
            u8 tsr_id = pcan->Vision_only_Sign_Type - 100;
            set_property_aliase_value("limit_value",(void*) limit_speed2[tsr_id]);
          } else {
            u8 tsr_id = pcan->Vision_only_Sign_Type;
            set_property_aliase_value("limit_value", (void*)limit_speed[tsr_id]);
          }
          return false;
        });
        // TODO:undefined yet
        mh.register_msg_handle(AEB_WARNING, [&](u8 *pbuff, int len) {
          struct GNU_DEF St_AEB_WARNING {
            u32 SoundType : 3;
            u32 rev0 : 29;
            u32 LDW_OFF : 1;
            u32 Left_LDW_ON : 1;
            u32 Right_LDW_ON : 1;
            u32 FCW_ON : 1;
            u32 rev1 : 5;
            u32 Peds_FCW : 1; // ped
            u32 PedsInDZ : 1;
            u32 rev2 : 24;
          };
          if (hud::capture_video)
            return false;
          static steady_clock::time_point ldw_tp = steady_clock::now();
          static steady_clock::time_point fcw_tp = steady_clock::now();
          const int duration_show = 5000;
          St_AEB_WARNING *pcan = (St_AEB_WARNING *)pbuff;

          hud::be_fcw = pcan->FCW_ON;
          if (pcan->FCW_ON) {
            printf("fcw_on~~~~~~~~~~~~\n");
            auto prt =
                static_cast<ft_rectangle_3d *>(hud::pmain_tar->get_child(0));
            prt->set_model_col({1.f, 0, 0});
          }

          if (pcan->Left_LDW_ON) {
            printf("left ldw on~~~~~~~~\n");
            ldw_tp = steady_clock::now();
            hud::pleft_lane->set_lane_color({1, 0, 0});
            hud::pleft_lane_trans->set_visible(true);
          }
          if (pcan->Right_LDW_ON) {
            printf("right ldw on~~~~~~~~\n");
            ldw_tp = steady_clock::now();
            hud::pright_lane->set_lane_color({1, 0, 0});
            hud::pright_lane_trans->set_visible(true);
          }
          if (!pcan->Left_LDW_ON && !pcan->Right_LDW_ON &&
                (hud::pleft_lane_trans->is_visible() ||
                 hud::pright_lane_trans->is_visible())) {
                hud::pright_lane_trans->set_visible(true);
            hud::pright_lane->set_lane_color({1, 1, 1});
                hud::pleft_lane_trans->set_visible(true);
            hud::pleft_lane->set_lane_color({1, 1, 1});
            auto tp_now = steady_clock::now();
            auto dur_mills = duration_cast<milliseconds>(tp_now - ldw_tp);
            if (dur_mills.count() >= duration_show) {
              hud::pright_lane_trans->set_visible(false);
              hud::pleft_lane_trans->set_visible(false);
            }
          }

          if (pcan->Peds_FCW) {
            printf("peds fcw on~~~~~~~~\n");
            fcw_tp = steady_clock::now();
            auto prt = static_cast<ft_rectangle_3d *>(hud::pdes->get_child(0));
            prt->set_model_col({1.f, 0, 0});
          } else {
            auto prt = static_cast<ft_rectangle_3d *>(hud::pdes->get_child(0));
            auto tp_now = steady_clock::now();
            auto dur_mills = duration_cast<milliseconds>(tp_now - fcw_tp);
            if (dur_mills.count() >= duration_show) {
              prt->set_model_col({1, 0.455, 0});
            }
          }
          return false;
        });
        mh.register_msg_handle(LENS_HEIGHT, [&](u8 *pbuff, int len) {
          hud_cali::level = *pbuff;
          printf("from 123 %d\n", *pbuff);
          if (hud_cali::level > 20)
            hud_cali::level = 20;
          return false;
        });
    }
    // UPDATE: ivi navigation
    {
        reg_trans_handle("navi_ani_curl", [&](int from, int to) {
            if ((from == 2) && (to == 0)) {
                play_tran("navi_ani_curl", 0, 1, 1000);
            }
            });
        reg_trans_handle("road_surface", [&](int from, int to) {
            if (from == 1 && to == 2) {
                set_property_aliase_value_T("show_road_surface", false);
                set_property_aliase_value_T("show_navi_ani_curl", true);
                play_tran("navi_ani_curl", 2, 0, 0);
            }
                
            });
        mh.register_msg_handle(HUDSERVICE_NEXT_INTERSECTION_NAME_INFO,
            [&](u8 *pbuff, int len) {
                pbuff += 3;
                //printf("from navi 1\n");
                set_property_aliase_value_T("show_navigroup", true);
                return true;
            });
        mh.register_msg_handle(HUDSERVICE_NEXT_ICON_INFO, [&](u8 *pbuff, int len) {
            pbuff += 3;
            int navi_id = *pbuff;
            navi::ani_triger_type(navi_id);
            return true;
            });

        mh.register_msg_handle(
            HUDSERVICE_NAVIGATION_STATE_INFO, [&](u8 *pbuff, int len) {
                pbuff += 3;
                bool navi_begin = 1 == *pbuff;
                vg_print("navi state: %d m\n", navi_begin)
                    set_property_aliase_value("show_navigroup", &navi_begin);
                if (navi_begin == false) {
                    navi::cancel_navi_ani();
                }
                return true;
            });

        mh.register_msg_handle(
            HUDSERVICE_TARGET_TIME_INFO, [&](u8 *pbuff, int len) {
                pbuff += 3;
                int hour = pbuff[0];
                int min = pbuff[1];
                vg_print("$$$hour=%d,min=%d\n", hour, min)
                    char str_dm[50] = {0};
                if (hour > 0) {
                    sprintf(str_dm, "%dhour%dmin", hour, min);
                } else {
                    sprintf(str_dm, "%dmin", min);
                }
                set_property_aliase_value("time_remaining", str_dm);
                return true;
            });

        mh.register_msg_handle(HUDSERVICE_NEXT_CROSS_DISTANCE_INFO,
            [&](u8 *pbuff, int len) {
                pbuff += 3;
                int *pdistance = (int *)pbuff;
                int next_cross_dis = *pdistance;
                navi::navi_distance_update(next_cross_dis);
                return true;
            });
        mh.register_msg_handle(HUDSERVICE_TARGET_DISTANCE_INFO,
            [&](u8 *pbuff, int len) {
                pbuff += 3;
                int *pdistance = (int *)pbuff;
                vg_print("distance to destination is %d m\n", *pdistance)
                    char str_dm[50] = {0};
                if (*pdistance > 1000) {
                    int dis_km = (*pdistance + 50) / 1000;
                    int dis_hm = ((*pdistance + 50) % 1000) / 100;
                    if (dis_hm != 0) {
                        sprintf(str_dm, "%d.%dkm", dis_km, dis_hm);
                    } else {
                        sprintf(str_dm, "%dkm", dis_km);
                    }
                } else {
                    sprintf(str_dm, "%dm", *pdistance);
                }
                navi::trig_map_point(*pdistance);
                set_property_aliase_value("dis_remaining", str_dm);
                return true;
            });

        //TODO:from vechicle body
        mh.register_msg_handle(VechicleSpeedDisplay, [&](u8 *pbuff, int len) {
            u16 *pcan = (u16 *)pbuff;
            float fkm_speed = *pcan * 0.05625f;
            int km_speed = fkm_speed;
            float fdelta = fkm_speed - km_speed;
            if (fdelta > 0.5f) {
                km_speed++;
            }
            float vspeed_m_s = *pcan * 0.015625f;
            navi::vspeed_update(vspeed_m_s);//m/s
            vg_print("vechicle speed:%d\n", km_speed)
                char str_speed[50] = "";
            sprintf(str_speed, "%d", km_speed);
            set_property_aliase_value("speed_value", str_speed);
            return true;
            });
        mh.register_msg_handle(BCM_LightChimeReq, [&](u8 *pbuff, int len) {
            struct GNU_DEF BCM {
                u8 turn_left : 1;
                u8 rv : 2;
                u8 turn_right : 1;
                u8 rv1 : 4;
                u8 rv3 : 2;
                u8 position_light : 1;
                u8 rv4 : 1;
                u8 rv5[4];
            };
            return true;
            });
        mh.register_msg_handle(VCU_ModelControl, [&](u8 *pbuff, int len) {
            u8 *pcan = pbuff + 1;
            char *str_gear[5] = {" ","P","R","N","D"};
            u8 igear = *pcan & 0x1f;
            if (igear > 4)
                igear = 0;
            vg_print("gear value=%d,%s\n", igear, str_gear[igear])
                set_property_aliase_value("gear", str_gear[igear]);
            return true;
            });
        mh.register_msg_handle(STEERING_ANGLE, [&](u8 *pbuff, int len) {
            u8 tmp = pbuff[0];
            pbuff[0] = pbuff[1];
            pbuff[1] = tmp;
            s16 *pcan = (s16 *)pbuff;
            navi::steering(*pcan);
            return true;
            });
    }
}
//XXX: before update
void before_update() {
    navi::trig_ani();
    hud::cancel_warning_no_lane_change();
    debug_cnt++;
    debug_cnt %= debug_cirlce;
}