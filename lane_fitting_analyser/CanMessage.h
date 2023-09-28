#pragma once
#include <vector>
#include <mutex>
namespace can_message{
  void can_devices(int state);
  using u16 = unsigned short;
  struct can_sig {
    u16 can_id;
    u16 sig_id;
  };
  using can_data_head = std::vector<can_sig>;
  using can_data_list = std::vector<double>;
  struct can_data_update {
    can_data_head head;
    can_data_list data_list;
    std::mutex data_lock;
    void update_data(u16 can_id, u16 sig_id, double phvalue);
  };
  extern can_data_update g_can_data_update;
  void can_message_view();
}