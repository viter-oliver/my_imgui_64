// ImGui - standalone example application for GLFW + OpenGL 3, using
// programmable pipeline If you are new to ImGui, see examples/README.txt and
// documentation at the top of imgui.cpp. (GLFW is a cross-platform general
// purpose library for handling windows, inputs, OpenGL/Vulkan graphics context
// creation, etc.) (GL3W is a helper library to access OpenGL functions since
// there is no standard header to access modern OpenGL functions easily.
// Alternatives are GLEW, Glad, etc.)

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#if !defined(IMGUI_WAYLAND)
#include <GL/gl3w.h> // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#else
#include "../../deps/glad/glad.h"
#endif
#include "Resource.h"
#include "SOIL.h"
#include "ft_base.h"
#include "ft_essay.h"
#include "res_output.h"
#include "texture.h"
#include <Commdlg.h>
#include <GLFW/glfw3.h>
#include <ShlObj.h>
#include <codecvt>
#include <fstream>
#include <functional>
#include <tchar.h>
#include <windows.h>
#include <io.h>
string g_current_running_directory;
void listFiles( const char * dir )
{
     using namespace std;
     HANDLE hFind;
     WIN32_FIND_DATA findData;
     LARGE_INTEGER size;
     hFind = FindFirstFile( dir, &findData );
     if( hFind == INVALID_HANDLE_VALUE )
     {
          cout << "Failed to find first file!\n";
          return;
     }
     do
     {
          // ºöÂÔ"."ºÍ".."Á½¸ö½á¹û 
          if( strcmp( findData.cFileName, "." ) == 0 || strcmp( findData.cFileName, ".." ) == 0 )
               continue;
          if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )    // ÊÇ·ñÊÇÄ¿Â¼ 
          {
               cout << findData.cFileName << "\t<dir>\n";
          }
          else
          {
               size.LowPart = findData.nFileSizeLow;
               size.HighPart = findData.nFileSizeHigh;
               string font_name( findData.cFileName );
               string font_file = g_current_running_directory + font_name;
               cout << findData.cFileName << "\t" << size.QuadPart << " bytes\n";
               int idx;
               auto ft_u = g_pfont_face_manager->load_font( font_name, font_file, idx );
               ft_u->_char_count_c = sqrt( ft_u->_ft_face->num_glyphs );
               auto ch_rows = ft_u->_char_count_c;
               for( ;; ch_rows++ )
               {
                    auto txt_num = ch_rows*ft_u->_char_count_c;
                    if( txt_num >= ft_u->_ft_face->num_glyphs )
                    {
                         break;
                    }
               }
			   ft_u->_char_count_c += 60;
               ft_u->_char_count_r = ch_rows+20;
          }
     }
     while( FindNextFile( hFind, &findData ) );
     cout << "Done!\n";
}
static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}
static bool get_font_item(void *data, int idx, const char **out_str) {
  auto &ft_nm_list = g_pfont_face_manager->get_dic_fonts();
  *out_str = ft_nm_list[idx]->_name.c_str();

  return true;
}
// string g_current_run_path;
#include <windows.h>

int main(int argc, char *argv[]) {
  // Setup window
  string running_app = argv[0];
  g_pfont_face_manager = make_shared<font_face_manager>();
  g_current_running_directory =
      running_app.substr(0, running_app.find_last_of('\\') + 1);
  string find_str = g_current_running_directory + "*.TTF";
  listFiles(find_str.c_str());
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    return 1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
#if 0
	GLFWmonitor*  pmornitor = glfwGetPrimaryMonitor();
	const GLFWvidmode * mode = glfwGetVideoMode(pmornitor);
	int iw, ih;
	iw = mode->width;
	ih = mode->height;
#endif
  // GLFWwindow* window = glfwCreateWindow(iw, ih, "Graphics app", pmornitor,
  // NULL);//full screen
  GLFWwindow *window = glfwCreateWindow(1600, 1300, "Graphics app", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync
  gl3wInit();

  // Setup ImGui binding
  ImGui::CreateContext();
  // ImGuiIO& io = ImGui::GetIO(); //(void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard
  // Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable
  // Gamepad Controls
  ImGui_ImplGlfwGL3_Init(window, true);

  // Setup style
  // ImGui::StyleColorsLight();
  // ImGui::StyleColorsClassic();
  ImGui::StyleColorsDark();
  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'misc/fonts/README.txt' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f,
  // NULL, io.Fonts->GetGlyphRangesChinese());

  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f, NULL,
  // io.Fonts->GetGlyphRangesChinese());
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  // io.Fonts->AddFontFromFileTTF("D:\\Qt\\Qt5.6.2\\5.6\\Src\\qtbase\\lib\\fonts\\DejaVuSerif-BoldOblique.ttf",
  // 16.0f, NULL, io.Fonts->GetGlyphRangesChinese());

  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);
  /*	char buffer[MAX_PATH];
          GetCurrentDirectory(MAX_PATH, buffer);
          g_current_run_path = buffer;
          g_current_run_path += "\\";*/
  // ImVec2 edit_window_size = ImVec2()
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  shared_ptr<ft_essay> sptr_essay = make_shared<ft_essay>();

  wstring wstr_content;

  string cur_ebook_path;
  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application. Generally you may always pass all inputs
    // to dear imgui, and hide them from your application based on those two
    // flags.
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();
    static bool show_set = true;
    ImGui::Begin("Ebook game", &show_set, ImVec2(1500, 1000),
                 ImGuiWindowFlags_NoMove);
    ImGui::BeginChild("Setup", ImVec2(1500, 200), true,
                      ImGuiWindowFlags_NoMove);
    auto &ft_nm_list = g_pfont_face_manager->get_dic_fonts();
    static int _font_id = 0;
    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Game state:%s",
                       sptr_essay->game_state.c_str());
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Time consume:%d seconds",
                       sptr_essay->consume_seconds);

    ImGui::Combo("font:", &_font_id, &get_font_item, 0, ft_nm_list.size());

    if (ft_nm_list.size() > 0) {
      sptr_essay->set_font(ft_nm_list[_font_id]);
    }

    if (sptr_essay->is_gaming()) {
      if (ImGui::Button("restore")) {
        sptr_essay->load_content(wstr_content);
      }
    } else {
      if (ImGui::Button("...")) {
        OPENFILENAME ofn = {sizeof(OPENFILENAME)};
        ofn.hwndOwner = GetForegroundWindow();
        ofn.lpstrFilter = "valid file:\0*.txt\0\0";
        char strFileName[MAX_PATH] = {0};
        ofn.nFilterIndex = 1;
        ofn.lpstrFile = strFileName;
        ofn.nMaxFile = sizeof(strFileName);
        ofn.lpstrTitle = "select a txt please!";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (GetOpenFileName(&ofn)) {
          printf("open file%s\n", strFileName);
          cur_ebook_path = strFileName;
          ifstream fin;
          fin.open(cur_ebook_path, ios::binary);
          auto utf8ToWstring = [](const std::string &str) {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
            return strCnv.from_bytes(str);
          };
          if (fin.is_open()) {
            auto file_size = fin.tellg();
            fin.seekg(0, ios::end);
            file_size = fin.tellg() - file_size;
            fin.seekg(0, ios::beg);
            string str_buff;
            str_buff.resize((int)file_size + 1);
            fin.read(&str_buff[0], file_size);
            str_buff[file_size] = 0;

            fin.close();

            if (str_buff[0] == 0xff && str_buff[1] == 0xfe) // unicode
            {
              auto wsz = ((int)file_size - 2) / 2;
              wstr_content.resize(wsz);
              memcpy(&wstr_content[0], &str_buff[2], wsz * 2);
            } else if ((unsigned char)str_buff[0] == 0xef &&
                       (unsigned char)str_buff[1] == 0xbb) // utf 8 bom
            {

              wstr_content = utf8ToWstring(str_buff.substr(3));
            } else {
              wstr_content = utf8ToWstring(str_buff);
            }
            sptr_essay->load_content(wstr_content);
          }
        }
      }
      ImGui::SameLine();
      ImGui::Text("Ebook name:%s", cur_ebook_path.c_str());
    }

    if (ImGui::Button("Shuffle")) {
      sptr_essay->shuffle();
    }
    ImGui::SliderFloat("Line spacing", &sptr_essay->line_spacing, 10.f, 100.f);
    ImGui::SliderFloat("Horizontal margin", &sptr_essay->hmargin, 20.f, 100.f);
    ImGui::SliderFloat("Vertical margin", &sptr_essay->vmargin, 30.f, 100.f);
    ImGui::SliderInt("Font size", &sptr_essay->font_size, 20, 50);

    ImGui::EndChild();
    // ImGui::SetNextWindowSize( ImVec2( 800, 1000 ), ImGuiCond_FirstUseEver );

    ImGui::BeginChild("content", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove);
    //
    // auto wsz = ImGui::GetWindowSize();
    // ImGui::Text("windows sz:%f,%f",wsz.x,wsz.y);
    if (sptr_essay) {
      sptr_essay->draw();
    }
    ImGui::EndChild();
    ImGui::End();
    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplGlfwGL3_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();

  return 0;
}
