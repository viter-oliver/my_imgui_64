#include <vector>
#include <map>
#include <windows.h>
#include <stdio.h>
#include "can_dev_mg.h"
namespace can_devs {
  using namespace std;
  using mp_canmsg = map<UINT, can_handle>;
  mp_canmsg g_cur_canmsgs;
  void reg_can_handle(UINT can_id, can_handle& can_h) {
    g_cur_canmsgs[can_id] = can_h;
  }
  const int max_device_count = 50;
  VCI_BOARD_INFO pInfo[max_device_count];
  enum baud_def {
    bps_10k,
    bps_20k,
    bps_40k,
    bps_50k,
    bps_80k,
    bps_100k,
    bps_125k,
    bps_200k,
    bps_250k,
    bps_400k,
    bps_500k,
    bps_666k,
    bps_800k,
    bps_1000k,
    bps_33p33k,
    bps_66p66k,
    bps_83p33k,
    bps_cnt
  };
  struct {
    const char* name;
    int timing0, timing1;
  } baud_items[bps_cnt] = {
    {"10 Kpps",0x31,0x1c},
    {"20 Kpps",0x18,0x1c},
    {"40 Kbps",0x87,0xff},
    {"50 Kbps",0x09,0x1c},
    {"80 Kbps",0x83,0xff},
    {"100 Kbps",0x04,0x1c},
    {"125 Kbps",0x03,0x1c},
    {"200 Kbps",0x81,0xfa},
    {"250 Kbps",0x01,0x1c},
    {"400 Kbps",0x80,0xb6},
    {"500 Kbps",0x00,0x1c},
    {"666 Kbps",0x80,0xb6},
    {"800 Kbps",0x00,0x16},
    {"1000 Kbps",0x00,0x14},
    {"33.33 Kbps",0x09,0x6f},
    {"66.66 Kbps",0x04,0x6f},
    {"83.33 Kbps",0x03,0x6f},
  };
  const int can_channels = 2;
  struct {
    bool connected{ false };
    bool be_receiving{ false };
    int _dev_type{ VCI_USBCAN1 };//or VCU_USBCAN2
    int can_channel_cnt() {
      return _dev_type == VCI_USBCAN1 ? 1 : 2;
    }
    int baud_indeics[can_channels];
    int open_device(int dev_index) {
      DWORD Reserved = 0;
      if (VCI_OpenDevice(_dev_type, dev_index, Reserved) != 1) {
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
      for (int channel_id = 0; channel_id < can_channel_cnt(); channel_id++) {
        InitInfo->Timing0 = baud_items[baud_indeics[channel_id]].timing0;
        InitInfo->Timing1 = baud_items[baud_indeics[channel_id]].timing1;
        if (VCI_InitCAN(_dev_type, dev_index, channel_id, InitInfo) != 1) {
          printf("fail to init can%d of device:%d\n", channel_id, dev_index);
          return -1;
        }
        VCI_SetReference(21, dev_index, channel_id, 3, 0);//清除原来的滤波列表
        for (auto& flt : filters) {
          VCI_SetReference(21, dev_index, channel_id, 1, &flt);
        }
        if (VCI_StartCAN(_dev_type, dev_index, channel_id) != 1) {
          printf("fail to start can%d of device:%d\n", channel_id, dev_index);
          return -1;
        }
      }
      return 0;
    }
    int close_device(int dev_index) {
      if (VCI_CloseDevice(_dev_type, dev_index) != 1) {
        return -1;
      }
      return 0;
    }
  }devices_states[max_device_count];

  int device_num;
  void can_devices(int state)
  {
    device_num = VCI_FindUsbDevice2(pInfo);
  }
  int device_num_revieving = 0;

  void receive_msg() {
    VCI_CAN_OBJ CanObj[200];
    for (int dev_id = 0; dev_id < device_num; dev_id++) {
      auto& dev_s = devices_states[dev_id];
      for (int channel_id = 0; channel_id < dev_s.can_channel_cnt(); channel_id++) {
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
}