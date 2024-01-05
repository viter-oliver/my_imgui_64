#include <string>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "can_dev_hmi.h"
namespace can_devs {
  using namespace std;
    const char* get_type(int dev_type) {
      if (VCI_USBCAN1) {
        return "VCI_USBCAN1";
      }
      else {
        return "VCI_USBCAN2";
      }
    }
static struct { bool connect{ false }, be_recieving{ false }; } dec_working_stats[max_device_count];

  void can_devices_control() {
    ImGui::Begin("can devices");

    for (int ix = 0; ix < device_num; ix++) {
      ImGui::Text("can%d", ix);
      auto& bcan = devices_states[ix];
      string id = std::to_string(ix);
      ImGui::Text("device type:%s", get_type(bcan._dev_type));
      for (int ii = 0; ii < bcan._channel_cnt; ii++) {
        string cid = std::to_string(ii);
        string baud = "channel"+cid+"baud rate:#" + id;
        ImGui::Combo(baud.c_str(), &bcan.baud_indeics[ii], [](void*, int id, const char** out)->bool {
          *out = baud_items[id].name;
          return true;
        }, 0, bps_cnt);
      }
      string conn_str = "connect...#" + id;
      string discn_str = "disconnect#" + id;
      if (!dec_working_stats[ix].connect&&ImGui::Button(conn_str.c_str())) {
        dec_working_stats[ix].connect = open_device(ix)==0;
      }
      else if (ImGui::Button(discn_str.c_str())) {
        dec_working_stats[ix].connect = close_device(ix)==0;
      }
    }
    ImGui::End();
  }
  int any_valid_devices() {
    for (int ix = 0; ix < device_num; ix++) {
      auto& dev_w = dec_working_stats[ix];
      if (dev_w.connect) {
        return ix;
      }
    }
    return -1;
  }
}