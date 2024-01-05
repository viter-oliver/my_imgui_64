#include <thread>
#include <memory>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <windows.h>
#include <Commdlg.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <ShlObj.h>
#include <codecvt>
#include <fstream>
#include <functional>
#include <stdio.h>
#include <tchar.h>
#include "video_dev_mg.h"
#include "create_txt_dic.h"
#include "can_dev_hmi.h"
#include "fast-lzma2.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_impl_glfw_gl3.h"
static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}
using namespace std;
int main(int argc,char** argv){
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    return 1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(1500, 1300, "Lane fitting analyser", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  gl3wInit();
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  ImGui::CreateContext();
  ImGui_ImplGlfwGL3_Init(window, true);
  ImGui::StyleColorsDark();
  video_dev_init();
  glfwUsbDevicStateCallback([](int state, wchar_t* dev_name) {
    video_dev_enum();
    can_devs::can_devices(state);
  });
  int txt_w = 1920, txt_h = 720,block_len=80;
  vector<txt_dic::dic_uint> txt_dix_o;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();
    static bool show_video_cap = true;
    ImGui::Begin("setup of txt dictionary",0, ImGuiWindowFlags_NoMove);
    ImGui::SliderInt("width of txt:", &txt_w, 0, 2000);
    ImGui::SliderInt("height of txt", &txt_h, 0, 2000);
    ImGui::SliderInt("length of sample block:", &block_len, 0, 200);
    ImGui::End();
    ImGui::Begin("Creator of texture dictionary from video", &show_video_cap, ImVec2(750, 750),ImGuiWindowFlags_NoMove);
    operator_on_video_dev();
    ImGui::End();
    can_devs::can_devices_control();
    static bool show_pic_cap = true;
    ImGui::Begin("Creator of texture dictionary from picture", &show_pic_cap, ImVec2(750, 750), ImGuiWindowFlags_NoMove);
    if (ImGui::Button("Load new image..."))
    {
      OPENFILENAME ofn = { sizeof(OPENFILENAME) };
      //ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = GetForegroundWindow();
      ofn.lpstrFilter = "image file:\0*.png;*.bmp;*.jpg;*.jpeg;*.gif;*.dds;*.tga;*.psd;*.hdr\0\0";
      ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;

      ofn.hInstance = GetModuleHandle(NULL);
      char strFileName[MAX_PATH] = { 0 };
      ofn.nFilterIndex = 1;
      ofn.lpstrFile = strFileName;
      ofn.nMaxFile = sizeof(strFileName);
      ofn.lpstrTitle = "Loading image...";
      if (GetOpenFileName(&ofn))
      {
         string image_sample(strFileName);
         thread thd_create_txt_tb ([&] {
           create_txt_dic(image_sample, txt_w, txt_h, block_len, txt_dix_o, [](int step) {
             
           });
           auto valid_dev_id = can_devs::any_valid_devices();
           if (valid_dev_id!=-1) {
              atomic_bool receving = false;
              mutex wait_ack_for_send;
              condition_variable trig_send;
              const int id_req_send = 0x121;
              const int id_acc_send = 0x122;
              const int id_dic_send = 0x123;
              VCI_CAN_OBJ req_send_obj;
              req_send_obj.ID = id_req_send;
              can_devs::send_message(valid_dev_id, 0, &req_send_obj, 1);

              thread tb_rec([&] {
                can_devs::reg_can_handle(0x100,[&](VCI_CAN_OBJ* pcan_obj) { 
                  switch (pcan_obj->ID) {
                  case id_acc_send:
                    trig_send.notify_one();
                    break;
                  default:
                    break;
                  }
                });
                receving = true;
                while (receving) {
                  can_devs::receive_msg();
                }
              });
              unique_lock<mutex> lock(wait_ack_for_send);
              trig_send.wait(lock);
              auto buff_sz = txt_dix_o.size() * sizeof(txt_dic::dic_uint);
              auto pre_buff_sz = buff_sz / 2;
              char* dest = (char*)malloc(pre_buff_sz);
              auto press_sz = FL2_compress(dest, pre_buff_sz, &txt_dix_o[0], buff_sz,6);
              //transmit compress buffer of texture dictionary
              const int can_buff_size = 8;
              auto obj_cnt = press_sz / can_buff_size;
              auto obj_rem = press_sz % can_buff_size;
              if (obj_rem > 0) {
                obj_cnt++;
              } else {
                obj_rem = can_buff_size;
              }
              PVCI_CAN_OBJ pdev = new VCI_CAN_OBJ[obj_cnt];
              int ix = 0;
              for (; ix < obj_cnt-1; ix++) {
                pdev[ix].ID = id_dic_send;
                memcpy(pdev[ix].Data, dest, can_buff_size);
                dest += can_buff_size;
              }
              pdev[ix].ID = id_dic_send;
              pdev[ix].DataLen = obj_rem;
              memcpy(pdev[ix].Data, dest, obj_rem);
              const int max_count_send = 1000;
              auto send_cnt = obj_cnt / max_count_send;
              auto send_rm = obj_cnt % max_count_send;
              for (int ix = 0; ix < send_cnt; ix++) {
                 can_devs::send_message(valid_dev_id, 0, pdev, send_cnt);
                 pdev += ix;
              }
              if (send_rm>0) {
                can_devs::send_message(valid_dev_id, 0, pdev, send_rm);
              }
              delete[] pdev;
              free(dest);
              receving = false;
           }
        });
        
      }
    }
    ImGui::End();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }
 // destroyWindow(videoWindowName);
  ImGui_ImplGlfwGL3_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
  return 0;
}