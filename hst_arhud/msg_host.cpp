//
// Created by v_f_z on 2022/6/29.
//
#include <thread>
#include <string.h>
#include "msg_host.h"
using namespace std;
namespace msg_utility{
    int next_id(int cur_id,int id_range,int steps){
        cur_id+= steps;
        cur_id%=id_range;
        return cur_id;
    }

    void print_buff(u8* pcmd,u16 len){
        const char ch[0x10]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
        struct wc{
            u8 h:4;
            u8 l:4;
        };
        char *pstr=new char[len*2];
        for(int ix=0;ix<len;ix++){
            wc* pwc=(wc*)&pcmd[ix];
            pstr[ix*2]=ch[pwc->l];
            pstr[ix*2+1]=ch[pwc->h];
        }
        printf("~~~%s\n",pstr);
        delete[] pstr;

    }

    void msg_host::pick_valid_data(u8 *pbuffer, int len) {

        static int debug_cnt=0;
        if(debug_cnt==0){
           // printf("pick %d bytes cmd:",len);
            //print_buff(pbuffer,len);
        }
        auto& cur_cmd=_cmd_queque[_front_id];
        memcpy(cur_cmd,pbuffer,len);
        auto nid= next_id(_front_id,que_length);
        if(nid==_rear_id){
            printf("too fast.slow down please!!!!\n");
            int rid=_rear_id,fid=_front_id;
            printf("nid=%d _rear_id=%d _front_id=%d\n",nid,rid,fid);
            //this_thread::sleep_for(chrono::milliseconds(50));
            std::unique_lock<std::mutex> lock(_lock);
            _blocking = true;
            _command_full.wait(lock, [&] {return nid != _rear_id; });
            lock.unlock();
            _blocking = false;
        }
        _front_id=nid;
        debug_cnt++;
        debug_cnt%=100;
    }

    int msg_host::execute_cmd() {
        int need_refresh_cnt=0;
        int rid0=_rear_id;
        int execute_cnt=0;
        for (; _rear_id!=_front_id; _rear_id= next_id(_rear_id,que_length)) {
            u8* exe_cmd=_cmd_queque[_rear_id];
            u16* pcmd_tag=(u16*)exe_cmd;
            bool need_refresh=false;
            execute_cnt++;
            auto cmd_id=_dic_msg_handle.find(*pcmd_tag);
            if (cmd_id!=_dic_msg_handle.end()){
                if (!_blocking || cmd_id->second._initial_weight ) {
                    need_refresh=cmd_id->second._handle(exe_cmd+2,cmd_length-2);
                }
            } else if(!_blocking) {
                need_refresh=_batch_cmd_handle(exe_cmd,cmd_length);
            }
            if (need_refresh){
                need_refresh_cnt++;
            }
        }
        _command_full.notify_one();
        int rid=_rear_id;
       // printf("execute cnt=%d rid0=%d _rear_id=%d\n",execute_cnt,rid0,rid);
        return need_refresh_cnt;
    }
}