//
// Created by v_f_z on 2022/6/30.
//

#ifndef MY_APPLICATION_REGISTER_MSG_HOST_H
#define MY_APPLICATION_REGISTER_MSG_HOST_H
#include "msg_host.h"
#include "HudData.h"
void hud_inital();
void show_ctls_hud();
void reg_debug();
void register_msg_host(msg_utility::msg_host& mh);
void before_update();
namespace hud {
    void init_controls();
}
namespace navi{
    void init_controls();
}
#endif //MY_APPLICATION_REGISTER_MSG_HOST_H
