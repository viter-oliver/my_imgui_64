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

string g_current_running_directory;
const int max_path_len = 1024;
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
struct book_unit {
  string name;
  wstring content;
  int total_score=0;
  int final_score=0;
};
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
  GLFWwindow *window = glfwCreateWindow(1600, 1300, "Graphics app", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync
  gl3wInit();

  ImGui::CreateContext();

  ImGui_ImplGlfwGL3_Init(window, true);

  ImGui::StyleColorsDark();

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  shared_ptr<ft_essay> sptr_essay = make_shared<ft_essay>();

  vector<book_unit> book_list;
  auto load_file_2_content = [](string& file_name, wstring& content) {
    ifstream fin;
    fin.open(file_name, ios::binary);
    auto utf8ToWstring = [](const std::string& str,wstring& wstr) {
      std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
      wstr= strCnv.from_bytes(str);
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
        content.resize(wsz);
        memcpy(&content[0], &str_buff[2], wsz * 2);
      }
      else if ((unsigned char)str_buff[0] == 0xef &&
        (unsigned char)str_buff[1] == 0xbb) {// utf 8 bom
        utf8ToWstring(str_buff.substr(3),content);
      } else {
        utf8ToWstring(str_buff,content);
      }
    }
  };
  int cid = 0;
  bool finish_all_test = false;
  auto cur_content = [&]()->wstring& {return book_list[cid].content; };

  sptr_essay->set_trig([&](score_state gtriger) {
    auto& cur_book = book_list[cid];
    cur_book.final_score = scale_score[gtriger] * cur_book.total_score;
    int e_cnt = 0;
    while (book_list[cid].final_score == 0 && e_cnt < book_list.size()) {
      e_cnt++;
      cid++;
      cid %= book_list.size();
    }
    if (e_cnt == (book_list.size() - 1)) {
      finish_all_test = true;
    } else {
      sptr_essay->load_content(book_list[cid].content);
      finish_all_test = false;
    }
  });
  string cur_directory;
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

      if (!finish_all_test) {
        if (ImGui::Button("restore")) {
          sptr_essay->load_content(cur_content());
        }
      } else {
        
        if (ImGui::Button("...")) {
          OPENFILENAME ofn = {sizeof(OPENFILENAME)};
          ofn.hwndOwner = GetForegroundWindow();
          ofn.lpstrFilter = "valid file:\0*.txt\0\0";
          char strFileName[max_path_len] = {0};
          ofn.nFilterIndex = 1;
          ofn.lpstrFile = strFileName;
          ofn.nMaxFile = 1024;
          ofn.lpstrTitle = "select txts please!";
          ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT| OFN_EXPLORER;
          if (GetOpenFileName(&ofn)) {
            book_list.clear();
            printf("open file%s\n", strFileName);
            char* pstr = ofn.lpstrFile;
            cur_directory = ofn.lpstrFile;
            pstr += (cur_directory.length() + 1);
            while (*pstr) {
              book_list.push_back(book_unit());
              auto& cur_book = *(book_list.end() - 1);
              auto& book = cur_book.name;
              auto& content = cur_book.content;
              book = pstr;
              string file_name = cur_directory +"\\"+ book;
              load_file_2_content(file_name, content);
              pstr += (book.length() + 1);
            }
            float total_sz = 0.f;
            for (auto& bk : book_list) {
              total_sz += bk.content.size();
            }
            finish_all_test = false;
            for (auto& bk : book_list) {
              bk.total_score = bk.content.size()*100.f / total_sz;
            }
            sptr_essay->load_content(cur_content());
            //sptr_essay->shuffle();
          }
        }
        ImGui::SameLine();
        ImGui::Text("Ebook name:%s", cur_ebook_path.c_str());
      }

      ImGui::SliderFloat("Line spacing", &sptr_essay->line_spacing, 10.f, 100.f);
      ImGui::SliderFloat("Horizontal margin", &sptr_essay->hmargin, 20.f, 100.f);
      ImGui::SliderFloat("Vertical margin", &sptr_essay->vmargin, 30.f, 100.f);
      ImGui::SliderInt("Font size", &sptr_essay->font_size, 20, 50);

    ImGui::EndChild();
    // ImGui::SetNextWindowSize( ImVec2( 800, 1000 ), ImGuiCond_FirstUseEver );
    ImGui::BeginChild("content", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove);
    if (!finish_all_test) {
      sptr_essay->draw();
    } else {
      int final_score = 0;
      for (auto& bk : book_list) {
        final_score += bk.final_score;
      }
      ImGui::Text("your final score is %d!", final_score);
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
