#pragma once
namespace blf{
  void start_blf_resolve(const char* blf_path,void(*msg_handle)(unsigned char*,int));
}