#include <thread>
#include <memory>
#include <atomic>
#include <chrono>
#include <windows.h>
#include <Commdlg.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <ShlObj.h>
#include <codecvt>
#include <fstream>
#include <functional>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <tchar.h>
#include "CanMessage.h"
#include "DeviceEnumerator.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_impl_glfw_gl3.h"
static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}
using namespace std;
using namespace cv;
atomic_bool be_preview=false, be_recording = false, be_analysing=false;
struct { 
  int col_from=0, col_to=0, row_from=0, row_to=0;
  Point left_top() {
    return { col_from,row_from };
  }
  Point right_bottom() {
    return { col_to,row_to };
  }
  int width() {
    return col_to - col_from;
  }
  int height() {
    return row_to - row_from;
  }
} valid_range;
int video_w = 0, video_h = 0;
shared_ptr<VideoWriter> p_video_writer = nullptr;
shared_ptr<ofstream> p_orec_file = nullptr;

int video_index = 0;
vector<Device> video_devices;
DeviceEnumerator de;
int main(int argc, char *argv[]) {
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
  wprintf(L"hello opencv!\n");
  
  video_devices = de.getVideoDevicesMap();
  can_message::can_devices(1);
  glfwUsbDevicStateCallback([](int state, wchar_t* dev_name) {
     auto vds= de.getVideoDevicesMap();
     if (be_preview) {
       auto& cur_video_name = video_devices[video_index].deviceName;
       bool find_dev = false;
       for (auto& vd : vds) {
         if (vd.deviceName == cur_video_name) {
           find_dev = true;
           break;
         }
       }
       if (!find_dev) {//TODO:destroy video context

       }
     }
     video_devices = vds;
     can_message::can_devices(state);
  });
  
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();
    static bool show_set = true;
    ImGui::Begin("Recording & calculation", &show_set, ImVec2(750, 750),ImGuiWindowFlags_NoMove);
    {
      if (video_devices.size() > 0) {
        ImGui::Combo("video devices", &video_index, [](void*, int idx, const char** out)->bool {
          *out = video_devices[video_index].deviceName.c_str();
          return true;
          }, 0,  video_devices.size());
      }
      can_message::can_message_view();

      if (be_preview) {
        if (!be_recording) {
          ImGui::SliderInt("col from", &valid_range.col_from, 0, valid_range.col_to);
          ImGui::SameLine(100);
          ImGui::SliderInt("col to", &valid_range.col_to, valid_range.col_from, video_w);
          ImGui::VSliderInt("row from", {30,100},&valid_range.row_from, 0, valid_range.row_to);
          ImGui::VSliderInt("row to", {30,100},&valid_range.row_to, valid_range.row_from, video_h);
          
          if (ImGui::Button("Recording")) {
            OPENFILENAME sfn = { sizeof(OPENFILENAME) };
            sfn.hwndOwner = GetForegroundWindow();
            sfn.lpstrFilter = "recording file:\0*.mp4\0\0";
            sfn.lpstrDefExt = "mp4";
            char strFileName[MAX_PATH] = { 0 };
            sfn.nFilterIndex = 1;
            sfn.lpstrFile = strFileName;
            sfn.nMaxFile = sizeof(strFileName);
            sfn.lpstrTitle = "Save to";
            sfn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
            sfn.FlagsEx = OFN_EX_NOPLACESBAR;
            if (GetSaveFileName(&sfn)) {
              string rec_file_path = strFileName;
              string directory = rec_file_path.substr(0, rec_file_path.find_last_of('\\') + 1);
              string exetend_name = rec_file_path.substr(rec_file_path.find_last_of('.') + 1);
              if (exetend_name != "mp4") {
                rec_file_path += ".mp4";
              }
              string rev_lane_file = rec_file_path + ".rec";
              Size frameSize(valid_range.width(), valid_range.height());
              p_video_writer = make_shared<VideoWriter>(rec_file_path, CAP_OPENCV_MJPEG, 30, frameSize, true);
              p_orec_file = make_shared<ofstream>(rev_lane_file, ios::binary);
              auto& can_head = can_message::g_can_data_update.head;
              int sz_can_sig = sizeof(can_message::can_sig);
              p_orec_file->write((const char*)&can_head[0], sz_can_sig * can_head.size());
              be_recording = true;
            }
          }
        } else {
          if (ImGui::Button("Stop record...")) {
            be_recording = false;
            p_video_writer->release();
            p_video_writer = nullptr;
            p_orec_file->close();
            p_orec_file = nullptr;
          }
        }  
      } else {
          if(ImGui::Button("Preview")){
            be_preview = true;
            thread thd_opencv_cap([] {
              String videoWindowName = "video with lanes";
              namedWindow(videoWindowName);
              VideoCapture cap(video_index);
              auto dw = cap.get(CAP_PROP_FRAME_WIDTH);
              video_w = dw;
              auto dh = cap.get(CAP_PROP_FRAME_HEIGHT);
              video_h = dh;
           
              Size frameSize(dw, dh);
              cap.set(CAP_PROP_FRAME_WIDTH, dw);
              cap.set(CAP_PROP_FRAME_HEIGHT, dh);
              while (be_preview) {
                Mat frame;
                if (cap.read(frame)) {
                
                  //resizeWindow(videoWindowName, { 1920,720 });
                  if (be_recording) {
                 
                    p_video_writer->write(frame);
                    auto& can_data = can_message::g_can_data_update;
                    lock_guard<std::mutex> lock(can_data.data_lock);
                    auto data_len = sizeof(double) * can_data.data_list.size();
                    p_orec_file->write((const char*)&can_data.data_list[0], data_len);
                    p_orec_file->flush();
                  }
                  imshow(videoWindowName, frame);           
                }
                waitKey(1);
              }
              destroyWindow(videoWindowName);
              });
            thd_opencv_cap.detach();
          }
      }
      if (be_analysing) {
        if (ImGui::Button("Stop analysing")) {
          be_analysing = false;
        }
      } else {
        if (ImGui::Button("Analyse lane fitting video...")) {
          OPENFILENAME sfn = { sizeof(OPENFILENAME) };
          sfn.hwndOwner = GetForegroundWindow();
          sfn.lpstrFilter = "analysing file:\0*.mp4\0\0";
          sfn.lpstrDefExt = "mp4";
          char strFileName[MAX_PATH] = { 0 };
          sfn.nFilterIndex = 1;
          sfn.lpstrFile = strFileName;
          sfn.nMaxFile = sizeof(strFileName);
          sfn.lpstrTitle = "Open";
          sfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
          if (GetOpenFileName(&sfn)) {
            be_analysing = true;
            string rec_file_path = strFileName;
            string rec_iou_file = rec_file_path + ".iou";
            thread thd_opencv_ana([rec_file_path, rec_iou_file] {
              String videoWindowName = "Analysing video with lanes";
              namedWindow(videoWindowName);
              VideoCapture cap(rec_file_path);
              auto dw = cap.get(CAP_PROP_FRAME_WIDTH);
              auto dh = cap.get(CAP_PROP_FRAME_HEIGHT);
              video_w = dw;
              video_h = dh;
              valid_range.col_from = 0;
              valid_range.col_to = video_w;
              valid_range.row_from = 0;
              valid_range.row_to = video_h;
              Size frameSize(dw, dh);
              cap.set(CAP_PROP_FRAME_WIDTH, dw);
              cap.set(CAP_PROP_FRAME_HEIGHT, dh);
            
              ofstream oiou_file(rec_iou_file, ios::binary);
              Mat frame;
              cap.read(frame);
              int key_value = 0;
              while (be_analysing
                &&(key_value = waitKeyEx(1)) 
                && key_value != 'b') {
                rectangle(frame, valid_range.left_top(), valid_range.right_bottom(), { 0,0,255 });
                imshow(videoWindowName, frame);
                if (key_value == 'n') {
                  cap.read(frame);
                }
              }
              cap.set(CAP_PROP_POS_FRAMES, 0);
              while (be_analysing) {
                if (cap.read(frame)) {
                  //resizeWindow(videoWindowName, { 1920,720 });
                   auto cal_frame = frame({ valid_range.row_from,valid_range.row_to }, { valid_range.col_from,valid_range.col_to });

                  float iou = 1.0;
                  oiou_file.write((const char*)&iou, sizeof(float));               
                  rectangle(frame, valid_range.left_top(), valid_range.right_bottom(), { 0,0,255 });
                  imshow(videoWindowName, frame);
                }
                waitKey(1);
              }
              oiou_file.close();
              cap.release();
              destroyWindow(videoWindowName);
              });
            thd_opencv_ana.detach();
          }
        }
      }
    }
    ImGui::End();
    static bool show_statistics = false;
    ImGui::Begin("Statistics of lane fitting", &show_statistics, {1000,1300}, ImGuiWindowFlags_NoMove);
    {
    static vector<float> video_ious;
    static int frame_index = 0,cur_frame_index=0;
    static bool showing_a_video = false;
    if (!showing_a_video) {
      if (ImGui::Button("Loading Iou file...")) {
        OPENFILENAME sfn = { sizeof(OPENFILENAME) };
        sfn.hwndOwner = GetForegroundWindow();
        sfn.lpstrFilter = "analysing file:\0*.mp4\0\0";
        sfn.lpstrDefExt = "mp4";
        char strFileName[MAX_PATH] = { 0 };
        sfn.nFilterIndex = 1;
        sfn.lpstrFile = strFileName;
        sfn.nMaxFile = sizeof(strFileName);
        sfn.lpstrTitle = "Open";
        sfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (GetOpenFileName(&sfn)) {
          string rec_file_path = strFileName;
          string rec_iou_file = rec_file_path + ".iou";
          thread thd_opencv_ana([rec_file_path, rec_iou_file] {
            String videoWindowName = "Video with iou";
            namedWindow(videoWindowName);
            VideoCapture cap(rec_file_path);
          
          
            ifstream iiou_file(rec_iou_file, ios::binary);
            if (iiou_file.is_open()) {
              showing_a_video = true;
              frame_index = cur_frame_index = 0;
              auto frame_count = cap.get(CAP_PROP_FRAME_COUNT);
              video_ious.resize(frame_count);
              iiou_file.read((char*)&video_ious[0], sizeof(float)* frame_count);
              Mat frame;
              cap.read(frame);
              int key_value = 0;
              while ((key_value = waitKeyEx(1))
              && showing_a_video) {
                rectangle(frame, valid_range.left_top(), valid_range.right_bottom(), { 0,0,255 });
                imshow(videoWindowName, frame);
                if (frame_index!=cur_frame_index) {
                  cur_frame_index = frame_index;
                  cap.set(CAP_PROP_POS_FRAMES, cur_frame_index);
                  cap.read(frame);
                }
              }
            }
            video_ious.clear();
            cap.release();
            destroyWindow(videoWindowName);
            });
          thd_opencv_ana.detach();
        }
      }
    } else {
      if (ImGui::Button("Finish..")) {
        showing_a_video = false;
      }
      if (video_ious.size() > 0) {
        ImGui::PlotHistogram("IOU of video", [](void*, int i) {return video_ious[i]; },0,video_ious.size(),0,"IOU",0,1,{0,100});
        ImGui::SliderInt("Current frame", &frame_index, 0, video_ious.size());
      }
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