#pragma once
#include <functional>
#include <lcm/lcm-cpp.hpp>
#include "exlcm/HMI_Pilot.hpp"
namespace adas_data{
  enum{
    lane_right,
    lane_next_right,
    lane_left,
    lane_next_left,
    lane_cnt
  };
  
  struct adas_base
  {
    struct{
      char nLineType;
      int8_t nLineColor;
      bool bValid;
      float fLDWFlag;
      float C0,C1,C2,C3,nearV,farV;
    } detected_lanes[lane_cnt];
    struct{
      int8_t nType;
      int16_t nObjectID;
      bool bValid;
      float fRelX;
      float fRelY;
      float fHeading;
      float width;
      float length;
    } detatected_obstacles[10];
    struct 
    {
      short nFollowCarID;
      short nWarningLevel;
    } follows[3]; 
    int16_t nSetSpeedLimit;
    int8_t nLaneChange;
    int8_t nDriveMode;//0:manual; 1:pilot_ready; 2:pilot;3:parking_ready; 4:parking; 5:ACC
    int8_t nGear;
    float fSteeringAngle;
    float fSpeed;
  };//base_adas[fifo_len];
  using adas_data_handler=std::function<void(adas_base& padas_base)>;
  void register_adas_data_handler(adas_data_handler phandler);
  void pick_adas_data_from_HMI_Pilot(const exlcm::HMI_Pilot* msg);
  void execute_data();
}