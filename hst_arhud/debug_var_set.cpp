#include "debug_var_set.h"
#include "af_type.h"

#include <atomic>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#ifndef _WIN32
#include "uart.h"
#include <unistd.h>
#else
#include <windows.h>
#endif
namespace fifo_debuger {
using namespace std;
using mp_var_set = map<string, handle_var_set>;
mp_var_set _mp_var_set;
const char *fifo_path = "/media/fifo_hst";
atomic<unsigned int> _rear_id{0}, _front_id{0};
const int FF_BUFF_LEN = 0x100;
const int queque_len = 50;
char buff_queue[queque_len][FF_BUFF_LEN];
bool recieve_cmd = false;
int fd_fifo = 0;
handle_var_set general_handle;

void init_var_set_fifo() {
#ifdef WIN32
  thread th_fifo([&]() {
    HANDLE hPipe =
        CreateNamedPipe(TEXT("\\\\.\\Pipe\\mypipe"), PIPE_ACCESS_INBOUND,
                        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                        PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_NOWAIT, NULL);

    if (INVALID_HANDLE_VALUE == hPipe) {
      printf("Create Pipe Error(%d)\n", GetLastError());
    } else {
      printf("Waiting For Client Connection...\n");

      DWORD rlen = 0;
      while (true) {
        if (ConnectNamedPipe(hPipe, NULL) != NULL) {
          if (ReadFile(hPipe, buff_queue[_front_id], FF_BUFF_LEN, &rlen,
                       NULL) == false) {
            printf("Read data from client fault\n");
            // break;
          } else {
            buff_queue[_front_id][rlen - 2] = '\0';
            printf("recieve cmd:%s\n", buff_queue[_front_id]);
            unsigned int front_next_id = _front_id + 1;
            front_next_id %= queque_len;
            _front_id = front_next_id;
          }
          DisconnectNamedPipe(hPipe);
        }
      }
      CloseHandle(hPipe);
    }
  });
  th_fifo.detach();
#else
  int ret = -1;
  if (access(fifo_path, F_OK)) {
    ret = mkfifo(fifo_path, 0666);
    if (ret == -1) {
      printf("fail to create fifo:%s\n", fifo_path);
    }
  } else {
    ret = 1;
  }
  for (int ix = 0; ix < queque_len; ++ix) {
    buff_queue[ix][0] = '\0';
  }
  if (ret != -1) {
    thread th_fifo([&]() {
      fd_fifo = open(fifo_path, O_RDONLY | O_CLOEXEC);
      if (fd_fifo == -1) {
        printf("fail to open fifo:%s\n", fifo_path);
        return;
      }
      recieve_cmd = true;
      while (recieve_cmd) {
        wait_fd_read_eable(fd_fifo);
        int n = read(fd_fifo, buff_queue[_front_id], FF_BUFF_LEN);
        if (n > 0) {
          buff_queue[_front_id][n - 1] = '\0';
          printf("recieve cmd:%s\n", buff_queue[_front_id]);
          unsigned int front_next_id = _front_id + 1;
          front_next_id %= queque_len;
          _front_id = front_next_id;
        }
      }
      close(fd_fifo);
    });
    th_fifo.detach();
  }
#endif
}

void attach_var_handle(std::string var_name, handle_var_set hdl) {
  _mp_var_set[var_name] = hdl;
}

void reg_command_handle(handle_var_set hdl) { general_handle = hdl; }

int cmd_update() {
  int refresh_count = 0;
  for (; _rear_id != _front_id; ++_rear_id, _rear_id = _rear_id % queque_len) {
    auto &cur_buff = buff_queue[_rear_id];
    string str_buff(cur_buff);
    if (str_buff == "print_dg") {
      using uit = unsigned int;
      auto next_id = [](uit id) {
        if (id > 0)
          --id;
        else
          id = queque_len - 1;
        return id;
      };
      for (uit cur_id = next_id(_rear_id); cur_id != _rear_id;
           cur_id = next_id(cur_id)) {
        if (strlen(buff_queue[cur_id]) > 0)
          printf("%s\n", buff_queue[cur_id]);
      }
    } else {
      auto eq_pos = str_buff.find_last_of('=');
      if (eq_pos != -1) {
        string cmd_key = str_buff.substr(0, eq_pos);
        const auto &icmd = _mp_var_set.find(cmd_key);
        if (icmd != _mp_var_set.end()) {
          string str_value = str_buff.substr(eq_pos + 1);
          if (icmd->second(const_cast<char *>(str_value.c_str()))) {
            refresh_count++;
          }
        } else {
          if (general_handle) {
            general_handle(cur_buff);
          }
        }
      }
    }
  }
  return refresh_count;

  /*if(update_cmd)
  {
          update_cmd=false;
          //g_image_show->load_image_file("cat.png");
          if(show_image_path.size()>0)
          {
                  g_image_show->load_image_file((char*)show_image_path.c_str());

          }
  }*/
}
} // namespace fifo_debuger
