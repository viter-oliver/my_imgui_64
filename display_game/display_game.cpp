// ImGui - standalone example application for GLFW + OpenGL 3, using
// programmable pipeline If you are new to ImGui, see examples/README.txt and
// documentation at the top of imgui.cpp. (GLFW is a cross-platform general
// purpose library for handling windows, inputs, OpenGL/Vulkan graphics context
// creation, etc.) (GL3W is a helper library to access OpenGL functions since
// there is no standard header to access modern OpenGL functions easily.
// Alternatives are GLEW, Glad, etc.)
#include <chrono>
#include <windows.h>
#include <nlohmann/json.hpp>
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
#include "af_font_res_set.h"
#include <algorithm>
#include "functions.hpp"
string g_current_running_directory;
using namespace auto_future;
using namespace std;
using namespace chrono;
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
void utf8ToWstring(const std::string& str, wstring& wstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
  wstr = strCnv.from_bytes(str);
}
void wstringToUtf8(const wstring wstr, string& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
  str = strCnv.to_bytes(wstr);
}
/**
[
{
"name":"fun1",
"content":"xxxx",
"options":["a1","a2","a3","a4"],
"time_limit":10,
"answer":"a1"
}
]

*/
// string g_current_run_path;
struct base_term {
  virtual void draw(af_vec2& start_pos,af_vec2& end_pos) = 0;
};
struct wstr_item :base_term {
  wstring wcontent;
  ps_font_unit& pf_u;
  GLint fontSize;
  af_vec4 txt_col;
  void draw(af_vec2& start_pos, af_vec2& end_pos) {
    g_pfont_face_manager->draw_wstring(pf_u, fontSize,
      start_pos, end_pos, 1.f,
      wcontent, txt_col, 2000, 0, false);
  }
};
struct img_item :base_term {
  GLuint _texture_id;
  ImVec2 _size;
  void draw(af_vec2& start_pos, af_vec2& end_pos) {
    ImGui::Image((ImTextureID)_texture_id, _size);
  }
};

using sd_base_term = shared_ptr<base_term>;
using term_list = vector<sd_base_term>;
struct optional_button {
  string op_name;
  GLuint _texture_id;
  ImVec2 _size;
  optional_button(string& name, GLuint& txt, ImVec2& sz)
    :op_name(name), _texture_id(txt),_size(sz)
  { }
  bool select() {
    return ImGui::ImageButton((ImTextureID)_texture_id, _size);
  }
};
using sd_optional_button = shared_ptr<optional_button>;
using option_btn_list = vector<sd_optional_button>;
struct question_def {
  string name;
#if 1
  wstring
#else
  term_list
#endif
    content;

  option_btn_list options;
  string answer;
  bool got_it{false};
};
using sd_question_def = shared_ptr<question_def>;
using test_paper = vector<sd_question_def>;
test_paper cur_test_paper;
  //sd_question_def sel_item = nullptr;
int sel_id = 0;
const int str_len = 0x100;
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
  GLFWwindow *window = glfwCreateWindow(1600, 1300, "Graphics app", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync
  gl3wInit();

  // Setup ImGui binding
  ImGui::CreateContext();
  ImGui_ImplGlfwGL3_Init(window, true);
  ImGui::StyleColorsDark();
  ImGuiIO& io = ImGui::GetIO();
  string FZLanTingHeiS = g_current_running_directory + "SIMYOU.TTF";// "FZLanTingHeiS-R-GB.ttf";
  io.Fonts->AddFontFromFileTTF(FZLanTingHeiS.c_str(), 16.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  //vairiables
  string cur_test_paper_file;
  string cur_test_paper_path;
  bool be_testing = false;
  uint16_t cur_topic_idx = 0;
  int  _font_size = 20;
  int _play_limit = 20;//seconds
  UINT _comsume_time = 0;
  af_vec4 txt_col{1,0,0,1};
  steady_clock::time_point  _play_start, _unit_start;
  auto save_prj = [&] (bool update=false) {
    if (cur_test_paper.size() > 0) {
      bool be_rootless = cur_test_paper_file.empty();
      if (be_rootless) {
        OPENFILENAME sfn = { sizeof(OPENFILENAME) };
        sfn.hwndOwner = GetForegroundWindow();
        sfn.lpstrFilter = "test paper file:\0*.json\0\0";
        sfn.lpstrDefExt = "json";
        char strFileName[MAX_PATH] = { 0 };
        sfn.nFilterIndex = 1;
        sfn.lpstrFile = strFileName;
        sfn.nMaxFile = sizeof(strFileName);
        sfn.lpstrTitle = "Save to";
        sfn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
        sfn.FlagsEx = OFN_EX_NOPLACESBAR;
        if (GetSaveFileName(&sfn)) {
          cur_test_paper_file = strFileName;
          cur_test_paper_path = cur_test_paper_file.substr(0, cur_test_paper_file.find_last_of('\\') + 1);
        }
      }
      if (update && !be_rootless) {
        return;
      }
      ofstream fout;
      fout.open(cur_test_paper_file);
      if (fout.is_open()) {
        using namespace nlohmann;
        nlohmann::json jsn_test_paper;
        for (auto& squ : cur_test_paper) {
          auto& qu = *squ;
          json jqu;
          jqu["name"] = qu.name;
          string content;
          wstringToUtf8(qu.content, content);
          jqu["content"] = content;
          json jopts;
          for (auto& opt : qu.options) {
            jopts.push_back(opt->op_name);
          }
          jqu["option"] = jopts;
          jsn_test_paper.push_back(jqu);
        }
        fout << jsn_test_paper << std::endl;
      }
    }
  };
  //Main loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();
    static bool show_set = true;
    static bool show_edit = false;
    ImGui::Begin("edit", &show_edit, ImVec2(1500, 100));
      ImGui::Columns(2);
      ImGui::SetColumnWidth(0, 400);
      static char name_str[str_len] = {};
      static char content_str[str_len] = {};
      if (ImGui::Button("Add question")) {
        cur_test_paper.push_back(make_shared<question_def>());
        sel_id = cur_test_paper.size() - 1;
      }
      ImGui::ListBox("list of questions", &sel_id, [](void*, int, const char** out_str) ->bool{
        auto& cur_test = *cur_test_paper[sel_id];
        *out_str= cur_test.name.c_str();
        name_str[0] = '\0';
        if (!cur_test.name.empty()) {
          strcpy_s(name_str, cur_test.name.size(), cur_test.name.c_str());
        }
        if (!cur_test.content.empty()) {
          string str;
          wstringToUtf8(cur_test.content, str);
          strcpy_s(content_str, str.size(), str.c_str());
        }
        
        }, 0, cur_test_paper.size());

      ImGui::NextColumn();
      if (sel_id >= 0 && sel_id < cur_test_paper.size()) {
        auto& cur_question = *cur_test_paper[sel_id];
     
        ImGui::InputText("name", name_str, str_len);
        ImGui::InputTextMultiline("content", content_str, str_len);
        if (ImGui::Button("...")) {
          cur_question.name = name_str;
          utf8ToWstring(content_str, cur_question.content);
        }
        auto& options = cur_question.options;

        int ix = 0;
        while (ix < options.size()) {
          auto& opt = *options[ix];
          ImGui::Image((ImTextureID)opt._texture_id, opt._size);
          char nm[20]="del#";
          itoa(ix, nm+4, 10);
          if (ImGui::Button(nm)) {
            options.erase(options.begin() + ix);
          } else {
            ++ix;
          }
        }
        if (ImGui::Button("Add option..")) {
          save_prj(true);
          OPENFILENAME ofn = { sizeof(OPENFILENAME) };
          ofn.hwndOwner = GetForegroundWindow();
          ofn.lpstrFilter = "image file:\0*.png;*.bmp;*.jpg;*.jpeg;*.gif;*.dds;*.tga;*.psd;*.hdr\0\0";
          char strFileName[MAX_PATH] = { 0 };
          ofn.nFilterIndex = 1;
          ofn.lpstrFile = strFileName;
          ofn.nMaxFile = sizeof(strFileName);
          ofn.lpstrTitle = "select a image file please!";
          ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
          if (GetOpenFileName(&ofn)) {
            string file_path(strFileName);
            string img_file_name = file_path.substr(file_path.find_last_of('\\') + 1);
            string img_dir= file_path.substr(0, file_path.find_last_of('\\') + 1);
            string opt_path = cur_test_paper_path + cur_question.name;
            if (!directoryExist(opt_path.c_str())) {
              createDirectory(opt_path.c_str());
            }
            if (img_dir != opt_path) {
              string str_cmd = "copy ";
              str_cmd += img_file_name;
              str_cmd += " ";
              str_cmd += opt_path;
              system(str_cmd.c_str());
            }
            int tw, th;
            auto txt_id = TextureHelper::load2DTexture(file_path.c_str(), tw, th, GL_RGBA, GL_RGBA, SOIL_LOAD_RGBA);
            if (txt_id > 0) {
              auto opt_btn = make_shared<optional_button>(img_file_name, txt_id, ImVec2(tw, th));
              cur_question.options.push_back(opt_btn);
            }
          }

        }
        if (ImGui::Button("Save")) {
          save_prj();
        }
      }
    ImGui::End();

    ImGui::Begin("Set up", &show_set, ImVec2(1500, 1000),
      ImGuiWindowFlags_NoMove);

      auto& ft_nm_list = g_pfont_face_manager->get_dic_fonts();
      static int _font_id = 0;
      ImGui::Combo("font:", &_font_id, &get_font_item, 0, ft_nm_list.size());
      ImGui::SliderInt("Font size", &_font_size, 20, 50);
      ImGui::SliderInt("Unit time limit", &_play_limit, 20, 30);
      ImGui::ColorEdit4("text color", (float*)&txt_col, ImGuiColorEditFlags_RGB);
    ImGui::End();
    ImGui::Begin("topic");
      auto see_if_end = [&] {
        _unit_start = steady_clock::now();
        int e_cnt = 0;
        while (!(cur_test_paper[++cur_topic_idx]->got_it) && e_cnt++ < cur_test_paper.size()) {}
        if (e_cnt >= cur_test_paper.size()) {
          be_testing = false;
        }
      };
      if (ImGui::Button("load testpaper...")) {
        OPENFILENAME ofn = { sizeof(OPENFILENAME) };
        ofn.hwndOwner = GetForegroundWindow();
        ofn.lpstrFilter = "valid file:\0*.txt\0\0";
        char strFileName[MAX_PATH] = { 0 };
        ofn.nFilterIndex = 1;
        ofn.lpstrFile = strFileName;
        ofn.nMaxFile = sizeof(strFileName);
        ofn.lpstrTitle = "select a test paper please!";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (GetOpenFileName(&ofn)) {
          printf("open file%s\n", strFileName);
          cur_test_paper_file = strFileName;
          cur_test_paper_path = cur_test_paper_file.substr(0, cur_test_paper_file.find_last_of('\\') + 1);

          ifstream fin;
          nlohmann::json jsn_test_paper;
          fin.open(cur_test_paper_file);
        
          if (fin.is_open()) {
            jsn_test_paper << fin;
            auto jsz = jsn_test_paper.size();
            cur_test_paper.clear();
            cur_test_paper.resize(jsz);

            for (int ix = 0; ix < jsz; ix++) {
              auto& cur_t = *cur_test_paper[ix];
              auto& jcur_t = jsn_test_paper[ix];
              cur_t.name = jcur_t["name"];
              string content = jcur_t["content"];
  #if 1
              utf8ToWstring(content, cur_t.content);
  #else
              cur_t.content=
              vector<string> sub_content_list;
              spilt_str(content, sub_content_list, '#');
              auto str_2_base_term = [&](string& str)->sd_base_term {
                sd_base_term s_bt;
                if (str[0] == '^') {
                  string sub_str = str.substr(1);

                }
                else {
                  wstring wstr;
                  tf8ToWstring(str, wstr);u
                  s_bt = make_shared<wstr_item>(wstr, ft_nm_list[_font_id], _font_size, txt_col);
                }
              };
  #endif
              auto& joptions = jcur_t["option"];
              string opt_dir = cur_test_paper_path + cur_t.name + '\\';
              for (int ix = 0; ix < joptions.size(); ix++) {

                string opt = joptions[ix];
                string opt_path = opt_dir + opt;
                int tw, th;
                auto txt_id = TextureHelper::load2DTexture(opt_path.c_str(), tw, th, GL_RGBA, GL_RGBA, SOIL_LOAD_RGBA);
                if (txt_id > 0) {
                  auto opt_btn = make_shared<optional_button>(opt, txt_id, ImVec2(tw, th));
                  cur_t.options.push_back(opt_btn);
                }
              }

            }
            random_shuffle(jsn_test_paper.begin(), jsn_test_paper.end());
            be_testing = true;
            cur_topic_idx = 0;
            _comsume_time = 0;
            _play_start = _unit_start = steady_clock::now();
          }
        }
      }
      if (be_testing&&cur_test_paper.size() > 0) {
        auto& cur_paper = *cur_test_paper[cur_topic_idx];
        ImVec2 winpos = ImGui::GetWindowPos();
        af_vec2 draw_pos = { winpos.x, winpos.y };
        af_vec2 end_pos;
        int delta_height = 0;
  #if 1
        g_pfont_face_manager->draw_wstring(ft_nm_list[_font_id], _font_size,
          draw_pos, end_pos, 1.f,
          cur_paper.content, txt_col, 2000, 0, false);
  #else
        for (auto& citem : cur_paper.content) {
          citem->draw(draw_pos, end_pos);
          draw_pos = end_pos;
        }
  #endif
        for (auto& opt_btn : cur_paper.options) {
          if (opt_btn->select()) {
            cur_paper.got_it = opt_btn->op_name == cur_paper.answer;
            see_if_end();
          }
        }
        auto currentTime = steady_clock::now();
        auto unit_its = duration_cast<seconds>(currentTime - _unit_start);
        auto unit_sec = unit_its.count();
        if (unit_sec > _play_limit) {
          see_if_end();
        }
      
      } else {
        ImGui::Text("time consume:%d seconds", _comsume_time);
      }
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
