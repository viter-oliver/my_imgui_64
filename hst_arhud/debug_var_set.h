#pragma once
#include <functional>
#include <map>
#include <string>
using handle_var_set=std::function<bool(char*) >;
namespace fifo_debuger{
  void attach_var_handle(std::string var_name,handle_var_set hdl);
  void reg_command_handle(handle_var_set hdl);
  void init_var_set_fifo();
  int cmd_update();
}


