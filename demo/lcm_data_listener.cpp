
#include <assert.h>
#include <memory>
#include <string.h>
#include <vector>
#include <chrono>
#include "af_bind.h"
#include "af_timer.h"
#include "afg.h"
#include "lcm_data_listener.h"
extern af_timer g_timer;
extern int scene_width;
extern int scene_height;
namespace lcm_data_listener {
using namespace exlcm;
using namespace std;
using namespace auto_future;
using namespace adas_data;
using namespace chrono;
#include "lcm_value_def.h"

const int type_Obstacles_cnt = 10;
const int content_len = 50;
static char str_show[content_len] = {0};

/** server:
 * lcm-logplayer  --lcm-url=udpm://238.255.32.56:3256?ttl=1 lcmlog-2022-11-17.01
 **/

class Handler {
  ft_4_time_curve_3d *pl_lane = {nullptr};
  ft_4_time_curve_3d *pll_lane = {nullptr};
  ft_4_time_curve_3d *pr_lane = {nullptr};
  ft_4_time_curve_3d *prr_lane = {nullptr};
  //ft_trans* pdriving_model={nullptr};
  ft_rectangle_3d* pdriving_model = { nullptr };
  ft_trans *pvans[type_Obstacles_cnt];
  ft_trans *ppedestrians[type_Obstacles_cnt];
  ft_trans *pmotors[type_Obstacles_cnt];
  ft_trans *ptrucks[type_Obstacles_cnt];
  ft_trans *pcars[type_Obstacles_cnt];
  ft_trans *pright_change_lane = nullptr;
  ft_trans *pleft_change_lane = nullptr;
  ft_trans *pprepare_right_change_lane = nullptr;
  ft_trans *pprepare_left_change_lane = nullptr;
  ft_trans *pflash_change_lane = nullptr;
  ft_textblock* pcf_ll=nullptr;
  ft_textblock* pcf_l = nullptr;
  ft_textblock* pcf_r = nullptr;
  ft_textblock* pcf_rr = nullptr;
  uint8_t cur_van_id = {0};
  uint8_t cur_pedestrian_id = {0};
  uint8_t cur_motor_id = {0};
  uint8_t cur_truck_id = {0};
  uint8_t cur_car_id = {0};
  steady_clock::time_point pre_tm;
  const af_vec3 warning_color[warning_level_cnt] = {
      {0.780392170f, 0.780392170f, 0.780392170f},
      {0.898039222f, 0.756862760f, 0.756862760f},
      {0.941176474f, 0.725490212f, 0.725490212f},
  };
  const af_vec3 lane_color[lane_color_cnt] = {
      {1.f, 1.f, 1.f},
      {1.f, 1.f, 1.f},
      {1.f, 0.635294f, 0.f},
      {0.003922f, 0.525490f, 0.760784f},
  };
  const af_vec3 driveing_model_col[drive_mode_cnt]={
    {1.f, 1.f, 1.f},//manul
    {0.8f, 0.8f, 1.f},//pilot_ready
    {0.2f, 0.2f, 1.f},//pilot
    {0.2f, 0.2f, 0.8f},//parking_ready
    {0.2f, 0.2f, 0.6f},//parking
    {0.0f, 0.5f, 0.5f},//acc
  };
  struct follow_obj {
    int16_t fid;
    int16_t nWarningLevel;
  };
  vector<follow_obj> follow_id;
  int tmid_lange_change = 0;
  void hide_all_elements() {
    for (int ix = 0; ix < type_Obstacles_cnt; ++ix) {
      pvans[ix]->set_visible(false);
      ppedestrians[ix]->set_visible(false);
      pmotors[ix]->set_visible(false);
      ptrucks[ix]->set_visible(false);
      pcars[ix]->set_visible(false);
    }
    cur_van_id = cur_pedestrian_id = cur_motor_id 
        = cur_truck_id = cur_car_id = 0;
    pl_lane->set_visible(false);
    pll_lane->set_visible(false);
    pr_lane->set_visible(false);
    prr_lane->set_visible(false);
    follow_id.clear();
  }
  void set_obstacle_status(uint8_t obs_type, float x_off, float z_off,
                           float fHeading,float w,int16_t obj_id) {
    uint16_t WarningLevel = no_alarm;
    const float min_w=0.01f;
    for (auto &fo : follow_id) {
      if (fo.fid == obj_id) {
        WarningLevel = fo.nWarningLevel;
        if (WarningLevel >= warning_level_cnt) {
          WarningLevel = no_alarm;
        }
        break;
      }
    }
    auto &diff_color = warning_color[WarningLevel];
    switch (obs_type) {
    case unclassified: {
    } break;
    case unknownsmall: {//origin w=3.375942、h=3.502218、l=9.626824
      if (cur_van_id == type_Obstacles_cnt) {
        printf("no more van can be used!\n");
        return;
      }
      auto pvan = pvans[cur_van_id++];
      pvan->set_visible(true);
      pvan->set_translation_x(x_off)
          .set_translation_z(z_off)
          .set_rotation_y(fHeading);
      /*if (w>min_w) {
        auto sw=w/3.375942f;
        pvan->set_scale_x(sw)
            .set_scale_y(sw)
            .set_scale_z(sw);
      }*/
     
      ft_color_mesh *pnode = (ft_color_mesh *)pvan->get_child(1);
      pnode->set_diffuse(diff_color);
    } break;
    case bicycle:
    case motorbike: {//origin w=0.589833、h=1.453484、l=1.711607f 
      if (cur_motor_id == type_Obstacles_cnt) {
        printf("no more motor can be used!\n");
        return;
      }
      auto pmotor = pmotors[cur_motor_id++];
      pmotor->set_visible(true);
      pmotor->set_translation_x(x_off)
      .set_translation_z(z_off)
      .set_rotation_y(fHeading);
      /*if (w>min_w) {
        auto sw=w/0.589833f;
        pmotor->set_scale_x(sw)
            .set_scale_y(sw)
            .set_scale_z(sw);
      }*/
      ft_color_mesh *pnode = (ft_color_mesh *)pmotor->get_child(0);
      pnode->set_diffuse(diff_color);
    } break;
    case car: {//origin w= 174.982414f h= 139.655983  l=431.689453f
      if (cur_car_id == type_Obstacles_cnt) {
        printf("no more car can be used!\n");
        return;
      }
      auto pcar = pcars[cur_car_id++];
      pcar->set_visible(true);
      pcar->set_translation_x(x_off)
      .set_translation_z(z_off)
      .set_rotation_y(180 + fHeading);
      if(w>min_w){
        auto sw=w/174.982414f;
        pcar->set_scale_x(sw)
            .set_scale_y(sw)
            .set_scale_z(sw);
      }
      // auto pbody = pcar->get_child(0);
      ft_color_mesh *pnode = (ft_color_mesh *)pcar->get_child(0);
      pnode->set_diffuse(diff_color);
    } break;
    //case unknownbig:
    case truck: {//origin w=3.226566f、h=3.983975、l=11.573599f
      if (cur_truck_id == type_Obstacles_cnt) {
        printf("no more truck can be used!\n");
        return;
      }
      auto ptruck = ptrucks[cur_truck_id++];
      ptruck->set_visible(true);
      ptruck->set_translation_x(x_off)
      .set_translation_z(z_off)
      .set_rotation_y(fHeading);
      if(w>min_w){
        auto sw=w/3.226566f;
        ptruck->set_scale_x(sw)
            .set_scale_y(sw)
            .set_scale_z(sw);
      }
      
      ft_color_mesh *pnode = (ft_color_mesh *)ptruck->get_child(0);
      pnode->set_diffuse(diff_color);
    } break;
    case pedestrian: {//origin w=59.889215f，h=179.970936，l=45.0f
      if (cur_pedestrian_id == type_Obstacles_cnt) {
        printf("no more pedestrian can be used!\n");
        return;
      }
      auto ppedestrian = ppedestrians[cur_pedestrian_id++];
      ppedestrian->set_visible(true);
      ppedestrian->set_translation_x(x_off)
      .set_translation_z(z_off)
      .set_rotation_y(180 + fHeading);
      /*if (w>min_w) {
        auto sw=w/59.889215f;
        ppedestrian->set_scale_x(sw)
            .set_scale_y(sw)
            .set_scale_z(sw);
      }*/
      ft_color_mesh *pnode = (ft_color_mesh *)ppedestrian->get_child(0);
      pnode->set_diffuse(diff_color);
    } break;
    }
  }
  void set_lane_change(int8_t change) {
    if (change == prepare_change_left || change == prepare_change_right) {
      if (!g_timer.is_actived_timer_ex(tmid_lange_change)) {
        g_timer.active_timer_ex(tmid_lange_change, 500);
      }
    } else {
      g_timer.deactive_time_ex(tmid_lange_change);
      pprepare_left_change_lane->set_visible(false);
      pprepare_right_change_lane->set_visible(false);
    }
    switch (change) {
    case prepare_change_left: {
      // TODO:flash left lane change
      if (pflash_change_lane != pprepare_left_change_lane) {
        if (pflash_change_lane)
          pflash_change_lane->set_visible(false);
        pflash_change_lane = pprepare_left_change_lane;
      }
      printf("prepare change left\n");
    } break;
    case prepare_change_right: {
      // TODO:flash right lane change
      if (pflash_change_lane != pprepare_right_change_lane) {
        if (pflash_change_lane)
          pflash_change_lane->set_visible(false);
        pflash_change_lane = pprepare_right_change_lane;
      }
      printf("prepare change right\n");
    } break;
    case change_left: {
      // TODO: keep left lane change
      pleft_change_lane->set_visible(true);
      pright_change_lane->set_visible(false);
      printf("change left\n");
    } break;
    case change_right: {
      // TODO:keep right lane change
      pright_change_lane->set_visible(true);
      pleft_change_lane->set_visible(false);
      printf("change right\n");
    } break;
    default: {
      pright_change_lane->set_visible(false);
      pleft_change_lane->set_visible(false);

    } break;
    }
  }

public:
  Handler() {
    pre_tm=steady_clock::now();
    auto pscene = (ft_light_scene*)get_aliase_ui_control("show_scene");
    pscene->set_size(scene_width, scene_height);
    pscene->recreate_framebuff();
    auto pconfidence = get_aliase_ui_control("show_lane_confidence");
    pconfidence->set_base_posx(scene_width * 0.5);
    pconfidence->set_base_posy(scene_height - 150);
    pcf_ll = (ft_textblock*)pconfidence->get_child(0);
    pcf_l= (ft_textblock*)pconfidence->get_child(1);
    pcf_r = (ft_textblock*)pconfidence->get_child(2);
    pcf_rr = (ft_textblock*)pconfidence->get_child(3);
    auto id = 0;
    pl_lane = (ft_4_time_curve_3d *)pscene->get_child(id++);
    pll_lane = (ft_4_time_curve_3d *)pscene->get_child(id++);
    pr_lane = (ft_4_time_curve_3d *)pscene->get_child(id++);
    prr_lane = (ft_4_time_curve_3d *)pscene->get_child(id++);
    pdriving_model =(ft_rectangle_3d*)pscene->get_child(id++)->get_child(0);
#define INIT_VEHICLE(vn)                                                       \
  vn##[0] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[1] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[2] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[3] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[4] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[5] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[6] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[7] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[8] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[9] = (ft_trans *)pscene->get_child(id++);
    INIT_VEHICLE(pvans)
    INIT_VEHICLE(ppedestrians)
    INIT_VEHICLE(pmotors)
    INIT_VEHICLE(ptrucks)
    INIT_VEHICLE(pcars)
    pright_change_lane = (ft_trans *)pscene->get_child(id++);
    pleft_change_lane = (ft_trans *)pscene->get_child(id++);
    pprepare_right_change_lane = (ft_trans *)pscene->get_child(id++);
    pprepare_left_change_lane = (ft_trans *)pscene->get_child(id++);

    tmid_lange_change = g_timer.register_timer_ex([&](int tid) {
      bool be_show = tid % 2;
      if (pflash_change_lane) {
        pflash_change_lane->set_visible(be_show);
      }
    });
    hide_all_elements();
  }
  void handleMessage(const adas_data::adas_base &msg) {
    static int debug_cnt = 0;
    hide_all_elements();
    // g_timer.execute();
    auto speed_limit = msg.nSetSpeedLimit;
    str_show[0] = '\0';
    snprintf(str_show, content_len, "Set speed:%d km/h", speed_limit);
    set_property_aliase_value("set_speed", str_show);

    str_show[0] = '\0';
    auto fsteering_angle = msg.fSteeringAngle;
    snprintf(str_show, content_len, "Steering angle:%.2f", fsteering_angle);
    set_property_aliase_value("steering_angle", str_show);

    str_show[0] = '\0';
    auto fSpeed = msg.fSpeed;
    snprintf(str_show, content_len, "%.2fkm/h", fSpeed);
    set_property_aliase_value("speed", str_show);

    str_show[0] = '\0';
    static char *str_drivemode[drive_mode_cnt] = {
        "Manual", "Pilot ready", "Pilot", "Parking ready", "Parking", "ACC"};
    auto nDriveMode = msg.nDriveMode;
    if (nDriveMode >= drive_mode_cnt)
      nDriveMode = manual;
    snprintf(str_show, content_len, "%s", str_drivemode[nDriveMode]);
    set_property_aliase_value("driving_model", str_show);
    pdriving_model->set_model_col(driveing_model_col[nDriveMode]);

    auto nGear = msg.nGear;
    static char *str_gear[en_g_cnt] = {"", " ", " ", " "," ", "D", "N", "R","P"};
    str_show[0] = '\0';
    if (nGear >= en_g_cnt)
      nGear = en_inv;
    snprintf(str_show, content_len, "%s", str_gear[nGear]);
    set_property_aliase_value("gear", str_show);

    auto set_lane_status = [&](int8_t nLineType, int8_t nLaneColor, bool bValid,
                              float C0, float C1, float C2, float C3,
                              float nearV, float farV,float voffset,
                              ft_4_time_curve_3d *plane) {
      if (nLineType == dashed || nLineType == dlm_dashed_solid ||
          nLineType == dlm_solid_dashed) {
        plane->set_txt_obj("lane_dashed.png");
      } else if (nLineType == dlm_solid_solid) {
        plane->set_txt_obj("lane1.png");
      } else if (nLineType == dlm_dashed_dashed) {
        plane->set_txt_obj("lane2.png");
      } else {
        plane->set_txt_obj("lane_solid.png");
      }
      if (nLaneColor >= lane_color_cnt) {
        nLaneColor = undecided;
      }
      plane->link();
      if (bValid) {
        plane->set_visible(true);
        plane->set_coeff(-C0, -C1, -C2, -C3)
            .set_near(nearV)
            .set_far(farV)
            .set_lane_color(lane_color[nLaneColor])
            .set_voffset(voffset);
      }
    };
    auto set_lane_confidence = [&](bool bValid, float fLDWFlag,ft_textblock* ptext) {
        if (bValid) {
            ptext->set_visible(true);
            str_show[0] = '\0';
            snprintf(str_show, content_len, "%.2f", fLDWFlag);
            ptext->set_content(str_show);
        }  else {
            ptext->set_visible(false);
        }
    };
    const float voffset_max=0.06667f;
    const float vspan=6.6667f;//m
    const float voffset_p_m= voffset_max/vspan;//v/m
    const float s_2_m_ms=0.00027778f;
    auto rspeed = fSpeed * s_2_m_ms;//m/ms
    auto cur_tm=steady_clock::now();
    int delta=chrono::duration_cast<chrono::duration<int, std::milli>>( cur_tm - pre_tm).count();
    auto r_span=delta * rspeed;//span of vehicle during the time;
    auto voffset_span=r_span*voffset_p_m;
    static float lane_voffset=0;
    lane_voffset+=voffset_span;
    if(lane_voffset>=voffset_max) lane_voffset=0.f;
    pre_tm=cur_tm;

    auto &left_ln = msg.detected_lanes[lane_left];
    set_lane_status(left_ln.nLineType, left_ln.nLineColor, left_ln.bValid,
                    left_ln.C0, left_ln.C1, left_ln.C2, left_ln.C3,
                    left_ln.nearV, left_ln.farV,lane_voffset, pl_lane);
    set_lane_confidence(left_ln.bValid, left_ln.fLDWFlag, pcf_l);

    auto &next_left_ln = msg.detected_lanes[lane_next_left];
    set_lane_status(next_left_ln.nLineType, next_left_ln.nLineColor,
                    next_left_ln.bValid, next_left_ln.C0, next_left_ln.C1,
                    next_left_ln.C2, next_left_ln.C3, next_left_ln.nearV,
                    next_left_ln.farV, lane_voffset,pll_lane);
    set_lane_confidence(next_left_ln.bValid, next_left_ln.fLDWFlag, pcf_ll);

    auto &right_ln = msg.detected_lanes[lane_right];
    set_lane_status(right_ln.nLineType, right_ln.nLineColor, right_ln.bValid,
                    right_ln.C0, right_ln.C1, right_ln.C2, right_ln.C3,
                    right_ln.nearV, right_ln.farV, lane_voffset,pr_lane);
    set_lane_confidence(right_ln.bValid, right_ln.fLDWFlag, pcf_r);

    auto &next_right_ln = msg.detected_lanes[lane_next_right];
    set_lane_status(next_right_ln.nLineType, next_right_ln.nLineColor,
                    next_right_ln.bValid, next_right_ln.C0, next_right_ln.C1,
                    next_right_ln.C2, next_right_ln.C3, next_right_ln.nearV,
                    next_right_ln.farV, lane_voffset,prr_lane);
    set_lane_confidence(next_right_ln.bValid, next_right_ln.fLDWFlag, pcf_rr);

    for (int ix = 0; ix < 3; ++ix) {
      if (msg.follows[ix].nFollowCarID != -1) {
        follow_id.push_back(
            {msg.follows[ix].nFollowCarID, msg.follows[ix].nWarningLevel});
      }
    }
    set_lane_change(msg.nLaneChange);

    if (debug_cnt == 0) {
      // printf("start: objectID,x_off,z_off,obj type,fHeading\n");
    }
    auto &obstacles = msg.detatected_obstacles;
    for (int ix = 0; ix < 10; ++ix) {
      auto &obs = obstacles[ix];
      if (obs.bValid) {
        auto x_off = -obs.fRelX;
        auto z_off = obs.fRelY;
        auto obs_type = (int8_t)obs.nType;
        auto fHeading = obs.fHeading * 57.296f;
        auto width = obs.width;
       // auto length = obs.length;
        if (debug_cnt == 0) {
          // printf("obj(%d,%f,%f,%d,%f)\n", obs.nObjectID,x_off, z_off,
          // obs_type, fHeading);
        }

        set_obstacle_status(obs_type, x_off, z_off, fHeading,width,obs.nObjectID);
      }
    }
    debug_cnt++;
    debug_cnt %= 20;
  }
};
using sd_lcm = shared_ptr<lcm::LCM>;
using sd_handler = shared_ptr<Handler>;
sd_lcm p_lcm_dev = nullptr;
sd_handler p_handler = nullptr;
class hanlder_message {
public:
  void handleMessage(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                     const HMI_Pilot *msg) {
    adas_data::pick_adas_data_from_HMI_Pilot(msg);
  }
} _handle_message;
void init() {
  p_lcm_dev = make_shared<lcm::LCM>("udpm://238.255.32.56:3256?ttl=1");
  assert(p_lcm_dev->good() &&
         "fail initialize lcm by udpm://238.255.32.56:3256?ttl=1");
  p_handler = make_shared<Handler>();
  adas_data::register_adas_data_handler([&](adas_data::adas_base &base_data) {
    p_handler->handleMessage(base_data);
  });
  p_lcm_dev->subscribe("hmi_data", &hanlder_message::handleMessage,
                       &_handle_message);
  printf("finish lcm init!\n");
}
int hanlde() { return p_lcm_dev->handle(); }
} // namespace lcm_data_listener