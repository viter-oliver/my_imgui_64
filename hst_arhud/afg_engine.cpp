#include "afg_engine.h"
#include "main.h"
#include <thread>
#ifndef _WIN32
//#include "uart.h"
#else
#include "scomm.h"
#endif
#include "adas_can_def.h"
#include "af_bind.h"
#include "af_timer.h"
#include "debug_var_set.h"
#include "msg_host.h"
#include "register_msg_host.h"
using namespace auto_future;
using namespace std;
using namespace msg_utility;
msg_host g_msg_host;
af_timer g_timer;
afg_engine::afg_engine(int argc, char **argv) : application(argc, argv)
{
    _screen_width = 1920;
    _screen_height = 1080;

    _win_width = 1920;
    _win_height = 720;

    _wposx = 0.f;
    _wposy = 0.f;
    //set_rotate_axis_pos(1920 * 0.5f, 1080 * 0.5f);
    //set_rotate_angle(3.1415);
}
void afg_engine::resLoaded()
{
    printf("resloaded");
    fifo_debuger::init_var_set_fifo();
    hud::init_controls();
    navi::init_controls();
    reg_debug();
    register_msg_host(g_msg_host);
    /* #Test Code Start */
    fifo_debuger::attach_var_handle("rec", [&](char *pcmd_buff) {
        g_msg_host.begin_recording(pcmd_buff);
        return true;
        });
    fifo_debuger::attach_var_handle("end_rec", [&](char *pcmd_buff) {
        g_msg_host.end_recording();
        return true;
        });
    /* #Test Code End */

    auto is_valid = [](u8 *pcmd)->bool {
        u8 v0 = pcmd[0];
        pcmd[0] = pcmd[1];
        pcmd[1] = v0;
        u16 cmd_tag = *(u16 *)pcmd;
        if (num_in_range(cmd_tag, Vehicle_frame_a1, Vehicle_frame_c)
            || num_in_range(cmd_tag, Vehicle_frame_b0, Vehicle_frame_b7)
            || num_in_range(cmd_tag, Pedestrian_frame_a, Pedestrian_frame_c)
            || num_in_range(cmd_tag, lane_frame_1_h, lane_frame_c)
            || num_in_range(cmd_tag, TSR_frame_A, TSR_frame_C)
            || cmd_tag == AEB_WARNING) {
            return true;
        }
        return false;
    };

    /* #Test Code Start */
    thread read_bin_file([&] {
        ifstream fin(_arg_list[0], ios::binary);
        if (fin.is_open()) {
            while (!fin.eof()) {
                int sleep_dur, buff_len;
                fin.read((char *)&sleep_dur, 4);
                fin.read((char *)&buff_len, 4);
                sleep_dur = sleep_dur < 200 ? sleep_dur : 20;
                this_thread::sleep_for(chrono::milliseconds(sleep_dur));
                //this_thread::sleep_for(chrono::milliseconds(sleep_dur));
                char *pbuff = new char[buff_len];
                fin.read(pbuff, buff_len);
                g_msg_host.pick_valid_data((u8 *)pbuff, buff_len);
                delete[] pbuff;

            }
            fin.close();
        }

        });
    read_bin_file.detach();
    /* #Test Code End */
    return;
    thread thd_uart_com([&] {
        const int read_buff_len = 0x200;
        const int param_buff_len = read_buff_len * 2;
        const u8 valid_cmd_len = 10;
        u8 read_buff[read_buff_len];
        u8 param_buff[param_buff_len];
        u16 rear_id{0}, front_id{0};
        union {
            u8 cmd_value[valid_cmd_len];
            u16 cmd_tag;
        } cmd_head;
        struct {
            u8 back_value[valid_cmd_len];
            u8 back_len = {0};//<valid_cmd_len;
        }cmd_back;
#ifdef _WIN32
        scomm seril_port;
        DWORD EventMask = EV_RXCHAR;
        COMSTAT ComStat;
        DWORD dwErrorFlags;
        BOOL bReadState = FALSE;
        DWORD n;
        auto hcomm = seril_port.open(2, 57600);
        if (hcomm != INVALID_HANDLE_VALUE) {
            /* #Test Code Start */
            while (true) {
                //printf("got %d bytes\n",n);
                if (WaitCommEvent(hcomm, &EventMask, NULL) && (EventMask & EV_RXCHAR)) {
                    n = 0;
                    ClearCommError(hcomm, &dwErrorFlags, &ComStat);
                    if (ComStat.cbInQue > 0) {
                        bReadState =
                            ::ReadFile(hcomm, read_buff, ComStat.cbInQue, &n, NULL);
                        if (bReadState && n > 0) {
                            /*printf("get frame:");

                            for (int ix = 0; ix < dwBytesRead; ix++)
                            {
                            printf("%0x ", pdata[ix]);
                            }
                            printf("\n");*/
                        } else
                            continue;
                    }
                } else {
                    Sleep(50);
                    continue;
                }
                /* #Test Code End */
#else
        int uart_fd = openport("/dev/ttyS3");
        if (uart_fd > 0) {
            setport(uart_fd, 57600, 8, 1, 'n');
            printf("enter loop of receive uart message!\n");
            while (true) {
                wait_fd_read_eable(uart_fd);
                int n = read(uart_fd, read_buff, read_buff_len);
#endif

#if 1
                auto st_len = valid_cmd_len - cmd_back.back_len;
                if (n >= st_len) {
                    memcpy(cmd_back.back_value + cmd_back.back_len, read_buff, st_len);
                    cmd_back.back_len = valid_cmd_len;
                } else {
                    memcpy(cmd_back.back_value + cmd_back.back_len, read_buff, n);
                    cmd_back.back_len += n;
                    //printf("~~~n=%d st_len=%d\n",n,st_len);
                    continue;
                }
                if (cmd_back.back_len == valid_cmd_len) {
                    if (is_valid(cmd_back.back_value)) {
                        g_msg_host.pick_valid_data(cmd_back.back_value, valid_cmd_len);
                    } else {
                        //print_buff(cmd_back.back_value,valid_cmd_len);
                    }
                }

                u8 *pcmd = read_buff + st_len;
                auto rm_len = n - st_len;

                u8 *pcmd_end = pcmd + rm_len;
                while (pcmd_end - pcmd >= valid_cmd_len) {
                    if (is_valid(pcmd)) {
                        g_msg_host.pick_valid_data(pcmd, valid_cmd_len);
                    } else {
                        //print_buff(pcmd,valid_cmd_len);
                    }
                    pcmd += valid_cmd_len;
                }
                cmd_back.back_len = pcmd_end - pcmd;
                memcpy(cmd_back.back_value, pcmd, cmd_back.back_len);
#else
                if (n == 0)
                    continue;
                auto nid = front_id + n;
                if (nid < param_buff_len) {
                    memcpy(param_buff + front_id, read_buff, n);
                    front_id = nid;
                } else {
                    auto remain_len = nid - param_buff_len;
                    auto leave_len = n - remain_len;
                    memcpy(param_buff + front_id, read_buff, remain_len);
                    memcpy(param_buff, read_buff + remain_len, leave_len);
                    front_id = leave_len;
                }

                for (; rear_id != front_id;) {
                    static bool picked_valid_data = false;
                    static u8 valid_data_idx = 2;
                    u8 cur_data = param_buff[rear_id];
                    auto n_id = next_id(rear_id, param_buff_len);
                    if (picked_valid_data) {
                        cmd_head.cmd_value[valid_data_idx++] = cur_data;
                        if (valid_data_idx == valid_cmd_len) {
                            valid_data_idx = 2;
                            picked_valid_data = false;
                            g_msg_host.pick_valid_data(cmd_head.cmd_value, valid_cmd_len);
                        }
                    } else {
                        cmd_head.cmd_value[0] = cur_data;
                        cmd_head.cmd_value[1] = param_buff[n_id];
                        if (is_valid(cmd_head.cmd_tag)) {
                            picked_valid_data = true;
                            n_id = next_id(n_id, param_buff_len);
                        }
                    }
                    rear_id = n_id;
                }
#endif
            }
        } else {
            printf("fail to open serial port:/dev/ttyS3\n");
        }
        });
    thd_uart_com.detach();

}
void afg_engine::onUpdate()
{
    before_update();
    g_msg_host.execute_cmd();
    g_timer.execute();
    fifo_debuger::cmd_update();
}
AFGUI_APP(afg_engine)