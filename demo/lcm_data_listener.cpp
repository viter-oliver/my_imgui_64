
#include <assert.h>
#include <lcm/lcm-cpp.hpp>
#include <memory>
#include <string.h>
#include <vector>
#include "af_timer.h"
#include "af_bind.h"
#include "afg.h"
#include "exlcm/HMI_Pilot.hpp"
#include "lcm_data_listener.h"
extern af_timer g_timer;
namespace lcm_data_listener {
using namespace exlcm;
using namespace std;
using namespace auto_future;
#include "lcm_value_def.h"

const int type_Obstacles_cnt = 5;
const int content_len = 50;
static char str_show[content_len] = {0};

/** server:
 * lcm-logplayer lcmlog-2022-11-17.01 --lcm-url=udpm://238.255.32.56:3256?ttl=1
 **/

class Handler {
  ft_4_time_curve_3d *pl_lane = {nullptr};
  ft_4_time_curve_3d *pll_lane = {nullptr};
  ft_4_time_curve_3d *pr_lane = {nullptr};
  ft_4_time_curve_3d *prr_lane = {nullptr};
  ft_trans *pvans[type_Obstacles_cnt];
  ft_trans *ppedestrians[type_Obstacles_cnt];
  ft_trans *pmotors[type_Obstacles_cnt];
  ft_trans *ptrucks[type_Obstacles_cnt];
  ft_trans *pcars[type_Obstacles_cnt];
  ft_trans *pright_change_lane=nullptr;  
  ft_trans *pleft_change_lane=nullptr;
  ft_trans* pprepare_right_change_lane=nullptr;
  ft_trans* pprepare_left_change_lane=nullptr;
  ft_trans* pflash_change_lane=nullptr;
  uint8_t cur_van_id = {0};
  uint8_t cur_pedestrian_id = {0};
  uint8_t cur_motor_id = {0};
  uint8_t cur_truck_id = {0};
  uint8_t cur_car_id = {0};

  const af_vec3 warning_color[warning_level_cnt] = {
      {0.874509811f, 0.780392170f, 0.780392170f},
      {0.898039222f, 0.756862760f, 0.756862760f},
      {0.941176474f, 0.725490212f, 0.725490212f},
  };
  struct follow_obj {
    uint16_t fid;
    uint16_t nWarningLevel;
  };
  vector<follow_obj> follow_id;
  int tmid_lange_change=0;
  void hide_all_elements() {
    for (int ix = 0; ix < type_Obstacles_cnt; ++ix) {
      pvans[ix]->set_visible(false);
      ppedestrians[ix]->set_visible(false);
      pmotors[ix]->set_visible(false);
      ptrucks[ix]->set_visible(false);
      pcars[ix]->set_visible(false);
    }
    cur_van_id = cur_pedestrian_id = cur_motor_id = cur_truck_id = cur_car_id =
        0;
    pl_lane->set_visible(false);
    pll_lane->set_visible(false);
    pr_lane->set_visible(false);
    prr_lane->set_visible(false);
    follow_id.clear();
  }
  void set_obstacle_status(uint8_t obs_type, float x_off, float z_off,
                           float fHeading, int16_t obj_id) {
    uint16_t WarningLevel = no_alarm;
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
        }break;
    case unknownsmall: {
        if (cur_van_id == type_Obstacles_cnt) {
            printf("no more van can be used!\n");
            return;
        }
        auto pvan = pvans[cur_van_id++];
        pvan->set_visible(true);
        pvan->set_translation_x(x_off);
        pvan->set_translation_z(z_off);
        pvan->set_rotation_y(fHeading);
        ft_color_mesh* pnode = (ft_color_mesh*)pvan->get_child(1);
        pnode->set_diffuse(diff_color);
        }break;
    case bicycle:
    case motorbike: {
      if (cur_motor_id == type_Obstacles_cnt) {
        printf("no more motor can be used!\n");
        return;
      }
      auto pmotor = pmotors[cur_motor_id++];
      pmotor->set_visible(true);
      pmotor->set_translation_x(x_off);
      pmotor->set_translation_z(z_off);
      pmotor->set_rotation_y(fHeading);
      ft_color_mesh *pnode = (ft_color_mesh *)pmotor->get_child(0);
      pnode->set_diffuse(diff_color);
    } break;
    case car: {
      if (cur_car_id == type_Obstacles_cnt) {
        printf("no more car can be used!\n");
        return;
      }
      auto pcar = pcars[cur_car_id++];
      pcar->set_visible(true);
      pcar->set_translation_x(x_off);
      pcar->set_translation_z(z_off);
      pcar->set_rotation_y(180 + fHeading);
      auto pbody = pcar->get_child(0);
      ft_color_mesh *pnode = (ft_color_mesh *)pbody->get_child(0);
      pnode->set_diffuse(diff_color);
    } break;
    case unknownbig:
    case truck: {
      if (cur_truck_id == type_Obstacles_cnt) {
        printf("no more truck can be used!\n");
        return;
      }
      auto ptruck = ptrucks[cur_truck_id++];
      ptruck->set_visible(true);
      ptruck->set_translation_x(x_off);
      ptruck->set_translation_z(z_off);
      ptruck->set_rotation_y(fHeading);
      ft_color_mesh *pnode = (ft_color_mesh *)ptruck->get_child(0);
      pnode->set_diffuse(diff_color);
    } break;
    case pedestrian: {
      if (cur_pedestrian_id == type_Obstacles_cnt) {
        printf("no more pedestrian can be used!\n");
        return;
      }
      auto ppedestrian = ppedestrians[cur_pedestrian_id++];
      ppedestrian->set_visible(true);
      ppedestrian->set_translation_x(x_off);
      ppedestrian->set_translation_z(z_off);
      ppedestrian->set_rotation_y(180 + fHeading);
      ft_color_mesh *pnode = (ft_color_mesh *)ppedestrian->get_child(0);
      pnode->set_diffuse(diff_color);
    } break;
    }
  }
  void set_lane_change(int8_t change){
    if(change<change_left
    &&!g_timer.is_actived_timer_ex(tmid_lange_change)){
      g_timer.active_timer_ex(tmid_lange_change,500);
    } else {
      g_timer.deactive_time_ex(tmid_lange_change);
      pprepare_left_change_lane->set_visible(false);
      pprepare_right_change_lane->set_visible(false);
    }
    switch(change){
      case prepare_change_left:{
        //TODO:flash left lane change
        if(pflash_change_lane!=pprepare_left_change_lane){
          pflash_change_lane->set_visible(false);
          pflash_change_lane=pprepare_left_change_lane;
        }
        
      }break;
      case prepare_change_right:{
        //TODO:flash right lane change
        if(pflash_change_lane!=pprepare_right_change_lane){
          pflash_change_lane->set_visible(false);
          pflash_change_lane=pprepare_right_change_lane;
        }
      }break;
      case change_left:{
        //TODO: keep left lane change
        pleft_change_lane->set_visible(true);
        pright_change_lane->set_visible(false);
      }break;
      case change_right:{
        //TODO:keep right lane change
        pright_change_lane->set_visible(true);
        pleft_change_lane->set_visible(false);
      } break;
      default:{
        pright_change_lane->set_visible(false);
        pleft_change_lane->set_visible(false);
      }break;
    }
  }
public:
  Handler() {
    auto pscene = get_aliase_ui_control("show_scene");
    auto id = 0;
    pl_lane = (ft_4_time_curve_3d *)pscene->get_child(id++);
    pll_lane = (ft_4_time_curve_3d *)pscene->get_child(id++);
    pr_lane = (ft_4_time_curve_3d *)pscene->get_child(id++);
    prr_lane = (ft_4_time_curve_3d *)pscene->get_child(id++);
#define INIT_VEHICLE(vn)                                                       \
  vn##[0] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[1] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[2] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[3] = (ft_trans *)pscene->get_child(id++);                               \
  vn##[4] = (ft_trans *)pscene->get_child(id++);
    INIT_VEHICLE(pvans)
    INIT_VEHICLE(ppedestrians)
    INIT_VEHICLE(pmotors)
    INIT_VEHICLE(ptrucks)
    INIT_VEHICLE(pcars)
    pright_change_lane = (ft_trans *)pscene->get_child(id++);
    pleft_change_lane = (ft_trans *)pscene->get_child(id++);
    pprepare_right_change_lane = (ft_trans *)pscene->get_child(id++);
    pprepare_left_change_lane = (ft_trans *)pscene->get_child(id++);
 
    tmid_lange_change=g_timer.register_timer_ex([&](int tid){
      bool be_show=tid%2;
      if(pflash_change_lane){
        pflash_change_lane->set_visible(be_show);
      }
    });
    hide_all_elements();
  }
  void handleMessage(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                     const HMI_Pilot *msg) {
    static int debug_cnt = 0;
    hide_all_elements();

    auto speed_limit = msg->path_planner.nSetSpeedLimit;
    str_show[0] = '\0';
    snprintf(str_show, content_len, "acc:%d km/h", speed_limit);
    set_property_aliase_value("set_speed", str_show);

    auto set_lane_status = [](const Lane_Line &lane_data,
                              ft_4_time_curve_3d *plane) {
#if 1
      if (lane_data.nLineType == solid) {
        plane->set_txt_obj("lane_solid.png");
      } else {
        plane->set_txt_obj("lane_dashed.png");
      }
      plane->link();
#endif
      if (lane_data.bValid) {
        plane->set_visible(true);
        auto nearV = lane_data.viewRangeStart * 100.f;
        auto farV = lane_data.viewRangeEnd * 100.f;
        plane
            ->set_coeff(lane_data.C0 * 0.5f, lane_data.C1 * 0.5f,
                        lane_data.C2 * 0.5f, lane_data.C3 * 0.5f)
            .set_near(nearV)
            .set_far(farV);
      }
    };
    auto &l_lane_data = msg->lane_lines.left_line.first_line;
    set_lane_status(l_lane_data, pl_lane);
    auto &ll_lane_data = msg->lane_lines.next_left_line.first_line;
    set_lane_status(ll_lane_data, pll_lane);
    auto &r_lane_data = msg->lane_lines.right_line.first_line;
    set_lane_status(r_lane_data, pr_lane);
    auto &rr_lane_data = msg->lane_lines.next_right_line.first_line;
    set_lane_status(rr_lane_data, prr_lane);

    auto &hmi_path_planner = msg->path_planner;
    for (int ix = 0; ix < 3; ++ix) {
      if (hmi_path_planner.nFollowCarID[ix] != -1) {
        follow_id.push_back({(uint16_t)hmi_path_planner.nFollowCarID[ix],
                             (uint16_t)hmi_path_planner.nWarningLevel[ix]});
      }
    }
    auto lane_change=hmi_path_planner.nLaneChange;
    set_lane_change(lane_change);

    auto &obstacles = msg->obstacles;
    if (debug_cnt == 0) {
        printf("start: objectID,x_off,z_off,obj type,fHeading\n");
    }
    for (int ix = 0; ix < 10; ++ix) {
      auto &obs = obstacles[ix];
      if (obs.bValid) {
        auto x_off = -obs.fRelX * 50.f;
        auto z_off = obs.fRelY * 50.f;
        auto obs_type = (int8_t)obs.nType;
        auto fHeading = obs.fHeading;
        
        if (debug_cnt == 0)
          printf("obj(%d,%f,%f,%d,%f)\n", obs.nObjectID,x_off, z_off,
                 obs_type, fHeading);
        set_obstacle_status(obs_type, x_off, z_off, fHeading, obs.nObjectID);
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

void init() {
  p_lcm_dev = make_shared<lcm::LCM>("udpm://238.255.32.56:3256?ttl=1");
  assert(p_lcm_dev->good() &&
         "fail initialize lcm by udpm://238.255.32.56:3256?ttl=1");
  p_handler = make_shared<Handler>();
  p_lcm_dev->subscribe("hmi_data", &Handler::handleMessage, p_handler.get());
  printf("finish lcm init!\n");
}
int hanlde() { return p_lcm_dev->handle(); }
} // namespace lcm_data_listener