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
#include <stdio.h>
#include <tchar.h>
#include "video_dev_mg.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_impl_glfw_gl3.h"
static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}
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
  });
  int txt_w = 1920, txt_h = 720,block_len=80;
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