#include "video_dev_mg.h"
#include "DeviceEnumerator.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_impl_glfw_gl3.h"
using namespace std;
vector<Device> video_devs;
static int dev_id = 0;
static bool be_capturing=false;
DeviceEnumerator dev_enumer;
void video_dev_init(){
  video_devs = dev_enumer.getVideoDevicesMap();
}
void video_dev_enum(){
  auto vds = dev_enumer.getVideoDevicesMap();
  auto& cur_video_name = video_devs[dev_id].deviceName;
  bool find_dev = false;
  for (auto& vd : vds) {
    if (vd.deviceName == cur_video_name) {
      find_dev = true;
      break;
    }
  }
  if (!find_dev) {//TODO:destroy video context

  }
  video_devs = vds;
}
void operator_on_video_dev(){
  if (video_devs.size() > 0) {
    ImGui::Combo("video devices", &dev_id, [](void*, int id, const char** out)->bool {
      *out = video_devs[dev_id].deviceName.c_str();
      return true;
      },0,video_devs.size());
    if (be_capturing&& ImGui::Button("disconnect")) {
      be_capturing = false;
    } else if (ImGui::Button("connect")) {
      be_capturing = true;
    }

  }
}