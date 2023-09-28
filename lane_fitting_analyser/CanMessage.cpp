#include <map>
#include <memory>
#include <fstream>
#include <condition_variable>
#include <windows.h>
#include <Commdlg.h>
#include <dbcppp/CApi.h>
#include <dbcppp/Network.h>
#include <ControlCAN.h>
#include "CanMessage.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
namespace can_message{
  using namespace std;
  void can_data_update::update_data(u16 can_id, u16 sig_id, double phvalue) {
    int sig_data_id = 0;
    for (; sig_data_id < head.size(); sig_data_id) {
      auto& can_sig = head[sig_data_id];
      if (can_id == can_sig.can_id && sig_id == can_sig.sig_id) {
        break;
      }
    }
    auto least_data_cnt = sig_data_id + 1;
    lock_guard<std::mutex> lock(data_lock);
    if (data_list.size() < least_data_cnt) {
      data_list.resize(least_data_cnt);
    }
    data_list[sig_data_id] = phvalue;
  }
  
  can_data_update g_can_data_update;

  struct can_msg_edit_unit{
    const dbcppp::IMessage* msg;
    bool* signale_sels;
    bool is_sel() {
      for (int ix = 0; ix < msg->Signals_Size(); ix++) {
        if (signale_sels[ix]) {
          return true;
        }
      }
      return false;
    }
    ~can_msg_edit_unit() {
      delete[] signale_sels;
    }
  };

  using sd_can_msg_edit_unit = shared_ptr<can_msg_edit_unit>;
  using mp_canmsg = map<uint64_t, sd_can_msg_edit_unit>;
  mp_canmsg g_cur_canmsgs;
  unique_ptr<dbcppp::INetwork> g_cur_net;

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
    mutex mtx_rec;
    int _dev_type{ VCI_USBCAN1 };//or VCU_USBCAN2
    int can_channel_cnt() {
        return _dev_type == VCI_USBCAN1 ? 1:2;
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
        if (msg_u.second->is_sel()) {
          auto can_id = msg_u.first;
          VCI_FILTER_RECORD filter = { 0,can_id,can_id };
          filters.push_back(filter);
        }
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
          printf("fail to init can%d of device:%d\n", channel_id,dev_index);
          return -1;
        }
        VCI_SetReference(21, dev_index, channel_id,3,0);//清除原来的滤波列表
        for (auto& flt : filters) {
          VCI_SetReference(21, dev_index, channel_id, 1, &flt);
        }
        if (VCI_StartCAN(_dev_type, dev_index, channel_id) != 1) {
          printf("fail to start can%d of device:%d\n", channel_id,dev_index);
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
  mutex mutex_rv_msg;
  condition_variable cd_rv;
  shared_ptr<thread> sd_thd_rec = nullptr;

  void receive_msg() {
    VCI_CAN_OBJ pCanObj[200];
    while (sd_thd_rec != nullptr) {
      unique_lock<mutex> lock(mutex_rv_msg);
      cd_rv.wait(lock, [&] {return device_num_revieving > 0; });
      for (int dev_id = 0; dev_id < device_num; dev_id++) {
        auto& dev_s = devices_states[dev_id];
        if (dev_s.mtx_rec.try_lock()) {
          if (dev_s.be_receiving) {
            for (int channel_id = 0; channel_id < dev_s.can_channel_cnt(); channel_id++) {
              auto nm = VCI_Receive(dev_s._dev_type, dev_id, channel_id, pCanObj, 200, 0);
              for (int ix = 0; ix < nm; ix++) {
                auto& co = pCanObj[ix];
                auto icanmsg = g_cur_canmsgs.find(co.ID);
                if (icanmsg != g_cur_canmsgs.end()) {
                  auto& msg_u=*icanmsg->second;
                  auto& sig_sels = msg_u.signale_sels;
                  //const dbcppp::ISignal* mux_sig = msg_u.msg->MuxSignal();
                  auto sig_sz = msg_u.msg->Signals_Size();
                  for (int ix = 0; ix < sig_sz&& sig_sels[ix]; ix++) {
                    auto& sig = msg_u.msg->Signals_Get(ix);
                    auto raw= sig.Decode(co.Data);
                    auto phs = sig.RawToPhys(raw);
                    printf("value of %s_%s is %f\n", msg_u.msg->Name().c_str(), sig.Name().c_str(), phs);
                    g_can_data_update.update_data(co.ID, ix, phs);
                  }
                }
              }
            }
          }
          dev_s.mtx_rec.unlock();
        }
      }
    }
    
  }
  void can_message_view(){
    ImGui::BeginChild("Can Message", { 0,500 }, true);
      if (ImGui::Button("LoadDbc...")) {
        OPENFILENAME sfn = { sizeof(OPENFILENAME) };
        sfn.hwndOwner = GetForegroundWindow();
        sfn.lpstrFilter = "dbc file:\0*.dbc\0\0";
        sfn.lpstrDefExt = "dbc";
        char strFileName[MAX_PATH] = { 0 };
        sfn.nFilterIndex = 1;
        sfn.lpstrFile = strFileName;
        sfn.nMaxFile = sizeof(strFileName);
        sfn.lpstrTitle = "Open";
        sfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (GetOpenFileName(&sfn)) {
          ifstream idbc(strFileName);
          g_cur_canmsgs.clear();
          g_cur_net.reset();
          g_cur_net = dbcppp::INetwork::LoadDBCFromIs(idbc);
          for (const auto& msg : g_cur_net->Messages()) {
            auto msg_unit = make_shared<can_msg_edit_unit>();
            msg_unit->msg = &msg;
            msg_unit->signale_sels=new bool[msg.Signals_Size()];
            memset(msg_unit->signale_sels, 0, sizeof(bool) * msg.Signals_Size());
            g_cur_canmsgs[msg.Id()] = msg_unit;
          }
        }
      }
      //can message list
      for (auto& msg : g_cur_canmsgs) {
        auto canid = msg.first;
        auto& msg_unit = *msg.second;
        if (ImGui::TreeNode((void*)(intptr_t)canid, "%s[%x]", msg_unit.msg->Name().c_str(),canid)){
          auto& signals = msg_unit.msg->Signals();
          
          int sig_id = 0;
          for (auto& sig : signals) {
            string sname = sig.Name();
            string stype;// = sig.ValueType() == dbcppp::ISignal::EValueType::Signed ? "S" : "U";
            
            switch (sig.ExtendedValueType()) {
            case dbcppp::ISignal::EExtendedValueType::Integer:
              if (sig.ValueType() == dbcppp::ISignal::EValueType::Signed) {
                stype += "Integer";
              } else {
                stype+="UInteger";
              }
              break;
            case dbcppp::ISignal::EExtendedValueType::Double:
              stype += "Double";
              break;
            case dbcppp::ISignal::EExtendedValueType::Float:
              stype += "Float";
              break;
            }
            sname += " ";
            sname += stype;
            bool be_sel=ImGui::Checkbox(sname.c_str(), (bool*)&msg_unit.signale_sels[sig_id]);
            {
              auto& head=g_can_data_update.head;
              if (be_sel) {
                bool inserted = false;
                can_sig cs = { canid,sig_id };
                for (auto it = head.begin(); it != head.end(); it++) {
                  auto& ch = *it;
                  unsigned int* pu = (unsigned int*)&ch;
                  unsigned int* pus = (unsigned int*)&cs;
                  if (*pu > *pus) {
                    head.emplace(it, cs);
                    inserted = true;
                    break;
                  }
                }
                if (!inserted) {
                  head.emplace_back(cs);
                }
              } else {
                for (int ix =0; ix < head.size(); ix++) {
                  auto& ch = head[ix];
                  if (ch.can_id == canid && ch.sig_id == sig_id) {
                    head.erase(head.begin()+ix);
                    auto& data_list = g_can_data_update.data_list;
                    data_list.erase(data_list.begin() + ix);
                    break;
                  }
                }
              }
            }
            sig_id++;
          }
          ImGui::TreePop();
        }
      }
    ImGui::EndChild();
    //canalyst-ii
    ImGui::BeginChild("Can Devices", { 0,200 }, true);
      if (sd_thd_rec == nullptr) {
        sd_thd_rec = make_shared<thread>(receive_msg);
      }
      for (int ix = 0; ix < device_num; ix++) {
      auto& dev_s = devices_states[ix];
      ImGui::Text("Device index:%d,Serial name:%s,HardwareVerision:V%d,HardwareType:%s", pInfo[ix].can_Num, pInfo[ix].str_Serial_Num, pInfo[ix].hw_Version, pInfo[ix].str_hw_Type);
      ImGui::SameLine();
      if (!dev_s.connected) {
        if (ImGui::Button("connect")
          && dev_s.open_device(ix)==0) {
          dev_s.connected = true;

        }
      } else {
        if (ImGui::Button("disconnect")
          && dev_s.close_device(ix) == 0) {
          dev_s.connected = false;
          if (dev_s.be_receiving) {
            lock_guard<std::mutex> lock(dev_s.mtx_rec);
            dev_s.be_receiving = false;;
            device_num_revieving--;
          }
        }
        if (dev_s.mtx_rec.try_lock()) {
          if (!dev_s.be_receiving) {
            if (ImGui::Button("recieve...")) {
              dev_s.be_receiving = true;
              device_num_revieving++;
              cd_rv.notify_one();
            }
          } else {
            if (ImGui::Button("stop...")) {
              dev_s.be_receiving = false;
              device_num_revieving--;
            }
          }
          dev_s.mtx_rec.unlock();
        }
        
      }
    }
    ImGui::EndChild();
  }
}