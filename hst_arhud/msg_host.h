//
// Created by v_f_z on 2022/6/29.
//

#ifndef MY_APPLICATION_MSG_HOST_H
#define MY_APPLICATION_MSG_HOST_H
#include <functional>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>
namespace msg_utility{
    int next_id(int cur_id,int id_range,int steps=1);
    using s8 = char;
    using s16 = short;
    using s32 = int;
    using u8 = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using msg_handle=std::function<bool(u8*,u32)>;
    using batch_cmd_handle=std::function<bool(u8*,u32)>;
    struct cmd_unit {
        msg_handle _handle;
        u8 _initial_weight{ 0 };
    };
    using dic_msg_handle=std::map<u16, cmd_unit>;
    void print_buff(u8* pcmd,u16 len);
    const int cmd_length=64;
    const int que_length=0x800;
    class msg_host {
        dic_msg_handle _dic_msg_handle;
        batch_cmd_handle _batch_cmd_handle;
        std::atomic_int _rear_id{0},_front_id{0};
        std::atomic_bool _blocking{ false };
        u8 _cmd_queque[que_length][cmd_length];
        std::mutex _lock;
        std::condition_variable _command_full;
    public:
        bool register_msg_handle(u16 msg,msg_handle mhandle,u8 init_weight=1){
            _dic_msg_handle[msg] = { mhandle,init_weight };
            return true;
        }

        bool register_batch_cmd_handle(batch_cmd_handle bhandle){
            _batch_cmd_handle=bhandle;
            return true;
        }
        bool is_blocking() {
            return _blocking;
        }
        void pick_valid_data(u8* pbuffer,int len);
        int execute_cmd();
    };
}



#endif //MY_APPLICATION_MSG_HOST_H
