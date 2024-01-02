#pragma once
#include <functional>
#include <ControlCAN.h>
namespace can_devs {

  using can_handle = std::function<void(VCI_CAN_OBJ*)>;
  void reg_can_handle(UINT can_id, can_handle& can_h);
  void can_devices(int state);
}