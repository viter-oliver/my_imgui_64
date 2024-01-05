#include <vector>
#include <map>
#include <windows.h>
#include <stdio.h>
#include "can_dev_mg.h"
namespace can_devs {
  using namespace std;
  using mp_canmsg = map<UINT, can_handle>;
  mp_canmsg g_cur_canmsgs;
  void reg_can_handle(UINT can_id, can_handle can_h) {
    g_cur_canmsgs[can_id] = can_h;
  }

  VCI_BOARD_INFO pInfo[max_device_count];
  
  base_can devices_states[max_device_count];
  int open_device(int dev_index) {
    auto& can_s = devices_states[dev_index];
    DWORD Reserved = 0;
    if (VCI_OpenDevice(can_s._dev_type, dev_index, Reserved) != 1) {
      printf("fail to open device:%d\n", dev_index);
      return -1;
    }
    vector<VCI_FILTER_RECORD> filters;
    for (auto& msg_u : g_cur_canmsgs) {
        auto can_id = msg_u.first;
        VCI_FILTER_RECORD filter = { 0,can_id,can_id };
        filters.push_back(filter);
    }
    VCI_INIT_CONFIG InitInfo[1];
    InitInfo->Filter = 0;
    InitInfo->AccCode = 0x80000008;
    InitInfo->AccMask = 0xFFFFFFFF;
    InitInfo->Mode = 2;
    for (int channel_id = 0; channel_id < can_s._channel_cnt; channel_id++) {
      InitInfo->Timing0 = baud_items[can_s.baud_indeics[channel_id]].timing0;
      InitInfo->Timing1 = baud_items[can_s.baud_indeics[channel_id]].timing1;
      if (VCI_InitCAN(can_s._dev_type, dev_index, channel_id, InitInfo) != 1) {
        printf("fail to init can%d of device:%d\n", channel_id, dev_index);
        return -1;
      }
      VCI_SetReference(21, dev_index, channel_id, 3, 0);//清除原来的滤波列表
      for (auto& flt : filters) {
        VCI_SetReference(21, dev_index, channel_id, 1, &flt);
      }
      if (VCI_StartCAN(can_s._dev_type, dev_index, channel_id) != 1) {
        printf("fail to start can%d of device:%d\n", channel_id, dev_index);
        return -1;
      }
    }
    return 0;
  }
  int close_device(int dev_index) {
    auto& can_s = devices_states[dev_index];
    if (VCI_CloseDevice(can_s._dev_type, dev_index) != 1) {
      return -1;
    }
    return 0;
  }
  int device_num;
  void can_devices(int state)
  {
    device_num = VCI_FindUsbDevice2(pInfo);
    for (int ix = 0; ix < device_num; ix++) {
      devices_states[ix]._channel_cnt = pInfo[ix].can_Num;
    }
  }
  int device_num_revieving = 0;

  void receive_msg() {
    VCI_CAN_OBJ CanObj[200];
    for (int dev_id = 0; dev_id < device_num; dev_id++) {
      auto& dev_s = devices_states[dev_id];
      for (int channel_id = 0; channel_id < dev_s._channel_cnt; channel_id++) {
        auto nm = VCI_Receive(dev_s._dev_type, dev_id, channel_id, CanObj, 200, 0);
        for (int ix = 0; ix < nm; ix++) {
          auto& co = CanObj[ix];
          auto icanmsg = g_cur_canmsgs.find(co.ID);
          if (icanmsg != g_cur_canmsgs.end()) {
            icanmsg->second(&co);
          }
        }
      }
    }
  }

  int send_message(int dev_index, int channel, VCI_CAN_OBJ* pcan_obj, int length) {
    auto& can_s = devices_states[dev_index];
    return VCI_Transmit(can_s._dev_type,dev_index, channel, pcan_obj, length);
  }
}