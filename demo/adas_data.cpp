#include <atomic>
#include <chrono>
#include <thread>
#include "adas_data.h"
namespace adas_data{
  using namespace std;
  const int que_len=0xff;
  adas_base que_adas_base[que_len];
  atomic_int16_t _rear_id{0},_front_id{0};
  adas_data_handler _phandler=nullptr;
  int next_id(int cur_id,int id_range,int steps=1){
        cur_id+= steps;
        return cur_id%id_range;
  }
  void register_adas_data_handler(adas_data_handler phandler){
    _phandler=phandler;
  }
  void pick_adas_data_from_HMI_Pilot(const exlcm::HMI_Pilot* msg){
    auto& cur_data_unit=que_adas_base[_front_id];
    
    //obstacle
    auto& obstacle = cur_data_unit.detatected_obstacles;
    auto& exclm_objstacle = msg->obstacles;
    for (int ix = 0; ix < 10; ++ix) {
        obstacle[ix].fHeading = exclm_objstacle[ix].fHeading;
        obstacle[ix].fRelX = exclm_objstacle[ix].fRelX;
        obstacle[ix].fRelY = exclm_objstacle[ix].fRelY;
        obstacle[ix].nType= exclm_objstacle[ix].nType;
        obstacle[ix].bValid = exclm_objstacle[ix].bValid;
        obstacle[ix].nObjectID= exclm_objstacle[ix].nObjectID;
        obstacle[ix].width=exclm_objstacle[ix].width;
        obstacle[ix].length=exclm_objstacle[ix].length;
    }

    //lanes
    auto& plane = cur_data_unit.detected_lanes;
    auto& exclm_lane = msg->lane_lines;
    auto& exclm_right_lane = exclm_lane.right_line.first_line;
    plane[lane_right].bValid = exclm_right_lane.bValid;
    plane[lane_right].C0 = exclm_right_lane.C0;
    plane[lane_right].C1 = exclm_right_lane.C1;
    plane[lane_right].C2 = exclm_right_lane.C2;
    plane[lane_right].C3 = exclm_right_lane.C3;
    plane[lane_right].nearV = exclm_right_lane.viewRangeStart;
    plane[lane_right].farV = exclm_right_lane.viewRangeEnd;
    plane[lane_right].nLineType = exclm_right_lane.nLineType;
    plane[lane_right].nLineColor = exclm_right_lane.nLineColor;
    plane[lane_right].fLDWFlag= exclm_right_lane.fLDWFlag;
    
    auto& exclm_next_right_lane = exclm_lane.next_right_line.first_line;
    plane[lane_next_right].bValid = exclm_next_right_lane.bValid;
    plane[lane_next_right].C0 = exclm_next_right_lane.C0;
    plane[lane_next_right].C1 = exclm_next_right_lane.C1;
    plane[lane_next_right].C2 = exclm_next_right_lane.C2;
    plane[lane_next_right].C3 = exclm_next_right_lane.C3;
    plane[lane_next_right].nearV = exclm_next_right_lane.viewRangeStart;
    plane[lane_next_right].farV = exclm_next_right_lane.viewRangeEnd;
    plane[lane_next_right].nLineType = exclm_next_right_lane.nLineType;
    plane[lane_next_right].nLineColor = exclm_next_right_lane.nLineColor;
    plane[lane_next_right].fLDWFlag = exclm_next_right_lane.fLDWFlag;

    auto& exclm_left_lane = exclm_lane.left_line.first_line;
    plane[lane_left].bValid = exclm_left_lane.bValid;
    plane[lane_left].C0 = exclm_left_lane.C0;
    plane[lane_left].C1 = exclm_left_lane.C1;
    plane[lane_left].C2 = exclm_left_lane.C2;
    plane[lane_left].C3 = exclm_left_lane.C3;
    plane[lane_left].nearV = exclm_left_lane.viewRangeStart;
    plane[lane_left].farV = exclm_left_lane.viewRangeEnd;
    plane[lane_left].nLineType = exclm_left_lane.nLineType;
    plane[lane_left].nLineColor = exclm_left_lane.nLineColor;
    plane[lane_left].fLDWFlag = exclm_left_lane.fLDWFlag;

    auto& exclm_next_left_lane = exclm_lane.next_left_line.first_line;
    plane[lane_next_left].bValid = exclm_next_left_lane.bValid;
    plane[lane_next_left].C0 = exclm_next_left_lane.C0;
    plane[lane_next_left].C1 = exclm_next_left_lane.C1;
    plane[lane_next_left].C2 = exclm_next_left_lane.C2;
    plane[lane_next_left].C3 = exclm_next_left_lane.C3;
    plane[lane_next_left].nearV = exclm_next_left_lane.viewRangeStart;
    plane[lane_next_left].farV = exclm_next_left_lane.viewRangeEnd;
    plane[lane_next_left].nLineType = exclm_next_left_lane.nLineType;
    plane[lane_next_left].nLineColor = exclm_next_left_lane.nLineColor;
    plane[lane_next_left].fLDWFlag = exclm_next_left_lane.fLDWFlag;
    //follow
    auto& follows = cur_data_unit.follows;
    for (int ix = 0; ix < 3; ++ix) {
        follows[ix].nFollowCarID = msg->path_planner.nFollowCarID[ix];
        follows[ix].nWarningLevel = msg->path_planner.nWarningLevel[ix];
    }
    cur_data_unit.nSetSpeedLimit= msg->path_planner.nSetSpeedLimit;
    cur_data_unit.nLaneChange = msg->path_planner.nLaneChange;
    cur_data_unit.nGear=msg->vehicle_info.nGear;
    cur_data_unit.nDriveMode=msg->vehicle_info.nDriveMode;
    cur_data_unit.fSteeringAngle=msg->vehicle_info.fSteeringAngle;
    cur_data_unit.fSpeed=msg->vehicle_info.fSpeed;
    auto nid = next_id(_front_id, que_len);
    if (nid == _rear_id) {
        printf("too fast.slow down please!!!!\n");
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    _front_id = nid;

  }
  void execute_data(){
      for (; _rear_id != _front_id; _rear_id = next_id(_rear_id, que_len)) {
          if (_phandler) {
              auto& handler_data = que_adas_base[_rear_id];
              _phandler(handler_data);
          }
      }
  }
}