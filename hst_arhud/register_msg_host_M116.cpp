//
// Created by v_f_z on 2022/6/30.
//
#include "register_msg_host.h"
#include <stdio.h>
#include <chrono>
#include "adas_can_def.h"
#include "af_bind.h"
#include "af_state_manager.h"
#include "afg.h"
#include "debug_var_set.h"
#include "enum_txt_res_name.h"
#include "navi_turn.h"
//#include "smoothor.hpp"
#include "af_timer.h"
#include "can_driver.h"

using namespace msg_utility;
using namespace auto_future;
using namespace std;
using namespace chrono;
extern af_timer g_timer;
static bool debug_on = false;
#define vg_print(...)                            \
    if (debug_on && 0 == debug_cnt) {            \
        printf("$$$%s:%d ", __func__, __LINE__); \
        printf(__VA_ARGS__);                     \
    }
int debug_cnt = 0;
int debug_cirlce = 15;
float angle_to_s = 0.0174533f;
// @INFO: warning info
namespace warn {
    //ft_image *pbsd; /*  #TBD  */
    ft_image *pfcta;
    ft_image *pfcw;
    ft_image *pldw_l;
    ft_image *pldw_r;

    void init_controls() {
        //pbsd = (ft_image *)get_aliase_ui_control("show_bsd"); /*  #TBD  */
        pfcta = (ft_image *)get_aliase_ui_control("show_fcta");
        pfcw = (ft_image *)get_aliase_ui_control("show_fcw");
        pldw_l = (ft_image *)get_aliase_ui_control("show_ldw_l");
        pldw_r = (ft_image *)get_aliase_ui_control("show_ldw_r");
    }

}  // namespace warn

// @INFO: icon info
namespace icon {
    enum iconid {
        ICONID_NON = 0, /* 000 : NON                            */
        ICONID_TSR,     /* 001 : TSR                            */
        ICONID_CAM,     /* 002 : Camera                         */
        ICONID_ACC,     /* 003 : ACC                            */
        ICONID_NOP,     /* 004 : NOP                            */
        ICONID_LCC,     /* 005 : LCC                            */
        ICONIDMAX
    };
#define AIP_CPU_ENDIAN_LIT               (1) /* AIP_CPU_ENDIAN     */
#define AIP_CPU_ENDIAN_BIG               (2)
#define AIP_CPU_ENDIAN                   (AIP_CPU_ENDIAN_LIT)
#define ICON_NUM                         (3)
#define REQ_NUM                          (static_cast<u8>(iconid::ICONIDMAX))
#define ICON_ARRAY_BYTE_NUM              (8)
#define ICON_ARRAY_SIZE                  ((REQ_NUM + (ICON_ARRAY_BYTE_NUM - 1)) / ICON_ARRAY_BYTE_NUM)
#define ICON_DEF_INDEX                   (0)
#define GID_NON                          (0x0FFFFFFF)
#define GID_MAX                          (en_gid_0power_bg_png)
    typedef union {
        u32 reqbit[ICON_ARRAY_SIZE] = {0};
        struct {
#if (AIP_CPU_ENDIAN == AIP_CPU_ENDIAN_LIT)
            /* data 0 */
            u32 id000 : 1;
            u32 id001 : 1;
            u32 id002 : 1;
            u32 id003 : 1;
            u32 id004 : 1;
            u32 id005 : 1;
            u32 reserve : 26;
#else
            u32 reserve : 26;
            u32 id005 : 1;
            u32 id004 : 1;
            u32 id003 : 1;
            u32 id002 : 1;
            u32 id001 : 1;
            u32 id000 : 1;
#endif
        } bit;
    } iconreq;

    /* @INFO: Draw ACC */
#define INVALID_ACCSPD                   (255) 

#define ICON_ACC_COL_NUM                 (3)
#define IOCN_ACC_IDX_GRAY                (0)
#define IOCN_ACC_IDX_BLUE                (1)
#define IOCN_ACC_IDX_YELLOW              (2)
#define IOCN_ACC_IDX_NON                 (IOCN_ACC_IDX_GRAY)

/* CAN signal ACC_State */
#define ICON_ACC_STS_GRAY_1              (2)
#define ICON_ACC_STS_GRAY_2              (3)
#define ICON_ACC_STS_BLUE_1              (4)
#define ICON_ACC_STS_BLUE_2              (5)

    ft_image *picon[3];
    ft_textblock *picon_val[3];

    const int ICON_GID[REQ_NUM] = {
        GID_NON,
        en_gid_0LCC_png,
        en_gid_0NOP_png,
        en_gid_0ACC_png,
        en_gid_0TSV_png,
        en_gid_0TSR_png,
    };
    const int ACC_GID[ICON_ACC_COL_NUM] = {
        en_gid_0ACC_png,
        en_gid_0ACC_png,
        en_gid_0ACC_png
    };
    const string ICON_TEXT_ALIAS[ICON_NUM] = {
        "icon1",
        "icon2",
        "icon3"
    };
    const string ICON_INFO_ALIAS[ICON_NUM] = {
        "icon_info1",
        "icon_info2",
        "icon_info3"
    };
    const string ICON_SHOW_ALIAS[ICON_NUM] = {
        "show_icon1",
        "show_icon2",
        "show_icon3"
    };
    const af_vec4 ICON_COL[ICON_ACC_COL_NUM] = {
        {0.8, 0.8, 0.8 },
        {0.f, 0.f, 1.f },
        {1.f, 1.f, 1.f }
    };

    bool accspd_visible = false;
    bool iconlcc_visble = false;
    bool iconnop_visble = false;
    bool iconacc_visble = false;
    bool icontsv_visble = false;
    bool icontsr_visble = false;
    char str_speed[50] = "";

    /* function */
    void init_controls();
    void properties(u32 *icon_group); /* judge level */
    iconreq get_req(void); /* get icon display requirement */
    void show_accspd(u8 speed);
    void jdg_iconacc_col_gb(u8 accsts);
    void jdg_iconacc_col_y(u8 accsts);

    /* function end */
    void show_icon(void) {
        u32 icon_group[ICON_NUM] = {
            GID_NON,
            GID_NON,
            GID_NON
        };
        properties(icon_group);

        /* show icon */
        for (int i = 0; i < ICON_NUM; i++) {
            if (icon_group[i] <= GID_MAX) {
                if (!(trans_is_playing("icon"))) {
                }
                set_property_txt_aliase_value(ICON_TEXT_ALIAS[i], icon_group[i]);
                set_property_aliase_value_T(ICON_SHOW_ALIAS[i], true);

            }
            else {
                set_property_aliase_value_T(ICON_SHOW_ALIAS[i], false);
            }
            switch (icon_group[i]) {
                case en_gid_0TSR_png:
                    picon_val[i]->set_visible(true);
                    break;
                case en_gid_0LCC_png:
                case en_gid_0NOP_png:
                case en_gid_0ACC_png:
                case en_gid_0TSV_png:
                default:
                    picon_val[i]->set_visible(false);
                    break;
            }
        }
    }
    iconreq get_req(void) {
        iconreq ret_req;

        ret_req.bit.id000 = 0;                 /* 000 : NON                            */
        ret_req.bit.id001 = iconlcc_visble;    /* 001 : LCC                            */
        ret_req.bit.id002 = iconnop_visble;    /* 002 : NOP                            */
        ret_req.bit.id003 = accspd_visible;    /* 003 : ACC                            */
        ret_req.bit.id004 = icontsv_visble;    /* 004 : TSV                            */
        ret_req.bit.id005 = icontsr_visble;    /* 005 : TSR                            */

        return ret_req;
    }
    /* @INFO: inital icon */
    void init_controls() {
        picon[0] = (ft_image *)get_aliase_ui_control("show_icon1");
        picon[1] = (ft_image *)get_aliase_ui_control("show_icon2");
        picon[2] = (ft_image *)get_aliase_ui_control("show_icon3");
        picon_val[0] = (ft_textblock *)picon[0]->get_child(0);
        picon_val[0]->set_visible(false);
        picon_val[1] = (ft_textblock *)picon[1]->get_child(0);
        picon_val[1]->set_visible(false);
        picon_val[2] = (ft_textblock *)picon[2]->get_child(0);
        picon_val[2]->set_visible(false);
    }

    /* acc speed */
    void show_accspd(u8 speed) {
        sprintf(str_speed, "%d", speed);
        accspd_visible = (speed != INVALID_ACCSPD);
    }
    /* acc speed end */

    /* acc */
    u8 acc_col_idx = IOCN_ACC_IDX_NON;
    void jdg_iconacc_col_gb(u8 accsts) {
        iconacc_visble = true;
        switch (accsts) {
            case ICON_ACC_STS_GRAY_1:
            case ICON_ACC_STS_GRAY_2:
                acc_col_idx = IOCN_ACC_IDX_GRAY;
                break;
            case ICON_ACC_STS_BLUE_1:
            case ICON_ACC_STS_BLUE_2:
                acc_col_idx = IOCN_ACC_IDX_BLUE;
                break;
            default:
                iconacc_visble = false;
                break;
        }
    }
    void jdg_iconacc_col_y(bool accsts) {
        if (accsts == true) {
            acc_col_idx = IOCN_ACC_IDX_YELLOW;
            iconacc_visble = true;
        }
    }
    /* acc end */
    /* @INFO: draw icon  */
        /* judge level  */
    void properties(u32 *icon_group) {
        u8 icon_idx, arr_cnt, info_cnt;
        u8 id[ICON_NUM] = {0};
        bool valid = false;
        iconreq req_msg = get_req();
        icon_idx = 0;
        static auto icon_idx_pre = icon_idx;
        for (int i = 0; i < ICON_NUM; i++) {
            icon_group[i] = GID_NON;
        }

        for (arr_cnt = 0; arr_cnt < ICON_ARRAY_SIZE; arr_cnt++) {
            for (info_cnt = 0; ((info_cnt < REQ_NUM) && (icon_idx < ICON_NUM)); info_cnt++) {
                valid = (req_msg.reqbit[arr_cnt] >> info_cnt) & 0x1;
                if (valid != false) {
                    icon_group[icon_idx] = ICON_GID[info_cnt];
                    icon_idx++;
                }
            }
        }

        if (icon_idx != icon_idx_pre) {
            play_tran("icon", icon_idx_pre, icon_idx);
        }
        icon_idx_pre = icon_idx;
    }


    void set_icon(void) {

    }
}
namespace adj {
#define BRI_LEL_NUM                      (0x15)          /* -10~10 : 0x00~0x14, unused 0x15~0x1F */
#define HT_LEL_NUM                       (0x15)          /* -10~10 : 0x00~0x14, unused 0x15~0x1F */
    ft_image *pbri;
    ft_image *pht;

    const signed char  bri_lv[BRI_LEL_NUM] = {
    -10, -9, -8, -7 ,-6 ,-5, -4, -3, -2, -1,
    0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    };
    const signed char  ht_lv[HT_LEL_NUM] = {
    -10, -9, -8, -7 ,-6 ,-5, -4, -3, -2, -1,
    0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    };
    void init_controls() {
        pbri = (ft_image *)get_aliase_ui_control("show_brightness");
        //pht = (ft_image *)get_aliase_ui_control("show_height");
    }
}
namespace navi {
    void init_controls() {}
}
namespace hud {
#define SPEED_MAX                        (220)           /* unit: km/h */

#define VEHSPDVD_DISABLE                 (0)  
#define VEHSPDVD_ENABLE                  (0)

#define VEHSPD_FACTOR                    (0.05625)
#define VEHSPD_DISABLE_PHY               (0.675)         /* physical_value 0.675 km/h same as 0x12 */
#define VEHSPD_DISABLE_RAW               (0x000C)
#define VEHSPD_ENABLE_PHY                (299.98125)     /* raw_value 0x14D5 same as 299.981 km/h  */
#define VEHSPD_ENABLE_RAW                (0x14D5)
#define VEHSPD_COEFF                     (1.03)          /* 1.03 km/h */
#define VEHSPD_DELTA                     (1.3)           /* 1.3 km/h */

    enum class car_t {
        CAR_8AT,
        CAR_PHEV
    };
    car_t car_type = car_t::CAR_8AT;
    char *str_gear[5] = {" ","P","R","N","D"};
    enum class gear_t {
        SHIFT_NON,
        SHIFT_P,
    };

    void init_controls() {
        set_property_aliase_value_T("show_2d", true);
        set_property_aliase_value_T("show_ar", false);
    }
}
namespace signal_time {
#define mrBMS_BatMsg                     (0x194)
#define srP_Msg1                         (0x20E)
#define srE_Msg1                         (0x249)
#define srTCU_GearInfo                   (0x266)
#define mrECM_SysStHMI2                  (0x329)
#define mrADU_BSD_Status                 (0x37F)
#define srBatCAN_Msg1                    (0x3A4)
#define HU_ICMCtrlReq                    (0x3F5)
#define HU_CarSet1                       (0x3FE)
#define mrADASCAN_F04                    (0x50B)
#define mrADASCAN_F05                    (0x50C)
#define mrADU_NOP_Info                   (0x50D)
#define HUD_Status                       (0x5E1)
#define SIG_CAST(x,y)                    (static_cast<x>(signal_time::can_idx::y))
    enum class can_idx {
        cid_mrBMS_BatMsg,
        cid_srP_Msg1,
        cid_srE_Msg1,
        cid_srTCU_GearInfo,
        cid_mrECM_SysStHMI2,
        cid_mrADU_BSD_Status,
        cid_srBatCAN_Msg1,
        cid_HU_ICMCtrlReq,
        cid_HU_CarSet1,
        cid_mrADASCAN_F04,
        cid_mrADASCAN_F05,
        cid_mrADU_NOP_Info,
        cid_HUD_Status,
        cid_MAX
    };
    struct st_SigTm {
        int timid;         /* time id                         */
        u32 per;           /* period   (ms)                   */
        u32 tot;           /* time out (timer_num)            */
    };
    st_SigTm sig_tm[SIG_CAST(u8, cid_MAX)] = {
        { 0, 10,  10 },    /* mrBMS_BatMsg                    */
        { 0, 20,  10 },    /* srP_Msg1                        */
        { 0, 50,  10 },    /* srE_Msg1                        */
        { 0, 20,  10 },    /* srTCU_GearInfo                  */
        { 0, 100, 10 },    /* mrECM_SysStHMI2                 */
        { 0, 50,  10 },    /* mrADU_BSD_Status                */
        { 0, 100, 10 },    /* srBatCAN_Msg1                   */
        { 0, 100, 10 },    /* HU_ICMCtrlReq                   */
        { 0, 100, 10 },    /* HU_CarSet1                      */
        { 0, 100, 10 },    /* mrADASCAN_F04                   */
        { 0, 100, 10 },    /* mrADASCAN_F05                   */
        { 0, 100, 10 },    /* mrADU_NOP_Info                  */
        { 0, 100, 10 }     /* HUD_Status                      */
    };
    void intial() {

        sig_tm[SIG_CAST(u8, cid_srP_Msg1)].timid = g_timer.register_timer_ex([&](int tid) {
            if (tid > sig_tm[SIG_CAST(u8, cid_srP_Msg1)].tot) {
                char *str = "---";
                set_property_aliase_value("speed_value", str);
            }
        });
        sig_tm[SIG_CAST(u8, cid_srE_Msg1)].timid = g_timer.register_timer_ex([&](int tid) {
            if (tid > sig_tm[SIG_CAST(u8, cid_srE_Msg1)].tot) {
                icon::iconacc_visble = false;
            }
        });
        sig_tm[SIG_CAST(u8, cid_HU_CarSet1)].timid = g_timer.register_timer_ex([&](int tid) {
            if (tid > sig_tm[SIG_CAST(u8, cid_HU_CarSet1)].tot) {

            }
        });
        sig_tm[SIG_CAST(u8, cid_mrADASCAN_F04)].timid = g_timer.register_timer_ex([&](int tid) {
            if (tid <= sig_tm[SIG_CAST(u8, cid_mrADASCAN_F04)].tot) {
                icon::accspd_visible = false;
            }
        });
        sig_tm[SIG_CAST(u8, cid_mrADASCAN_F05)].timid = g_timer.register_timer_ex([&](int tid) {
            if (tid > sig_tm[SIG_CAST(u8, cid_mrADASCAN_F05)].tot) {
                icon::iconacc_visble = false;
            }
        });
        sig_tm[SIG_CAST(u8, cid_HUD_Status)].timid = g_timer.register_timer_ex([&](int tid) {
            if (tid > sig_tm[SIG_CAST(u8, cid_HUD_Status)].tot) {

            }
        });

    }
}
void print_buff00(u8 *pbuff, int len) {
    static char num_tb[0x10] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    struct buff_data {
        u8 data_l : 4;
        u8 data_h : 4;
    };
    string pt_buff;
    while (len > 0) {
        buff_data *pdata = (buff_data *)pbuff++;
        len--;
        pt_buff += " x";
        pt_buff += num_tb[pdata->data_h];
        pt_buff += num_tb[pdata->data_l];
    }
    printf("%s\n", pt_buff.c_str());
}
// #define HUD_DEBUG
// intial
void hud_inital() {
    hud::init_controls();
    navi::init_controls();
    adj::init_controls();
    icon::init_controls();
    warn::init_controls();
    signal_time::intial();
}

// UPDATE:namespace hud

// UPDATE namespace navi
//  TODO register debug interface
static bool dis_visble = 0;
static bool road_visible = 0;
static bool lane_visible = 0;
void show_ctls_hud() {
    //set_property_aliase_value_T("show_main", true);
    //set_property_aliase_value_T("show_navi", true);

    //set_property_aliase_value_T("show_ar_road", true);
    //set_property_aliase_value_T("show_2d", true);
    //set_property_aliase_value("text_speed_val", "90");
    //set_property_aliase_value_T("show_ldw_r", true);
    set_property_aliase_value_T("show_calibration", true);
    //set_property_aliase_value_T("show_dis", dis_visble);
    //play_tran("fcta", 0, 1,100);
    //play_tran("ar_road", 0, 1,15);
    ////play_tran("ar_road", 0, 2, 15);
    //play_tran_playlist("2d2ar", 0, 2);
}
void reg_debug() {
    play_tran_playlist("a2", 0, 10);
    //fifo_debuger::attach_var_handle("play_road", [](char *pcmd_buff) -> bool {
    //    float cancel = atof(pcmd_buff);
    //    if (cancel == 1) {
    //        play_tran_playlist("a2",0,10);
    //    } else {
    //        cancel_play_tran("a2");
    //    }
    //    return true;
    //});
}
void register_msg_host(msg_utility::msg_host &mh) {
    // UPDATE register BCAN interface
    // UPDATE: ivi navigation
    mh.register_batch_cmd_handle([](u8 *pbuff, u32 len) -> bool {
        u16 *pcmd = (u16 *)pbuff;
        return false;
    });
    mh.register_msg_handle(srP_Msg1, [&](u8 *pbuff, int len) {
        const Singnal VehSpd = {8,15};
        const Singnal VehSpdVd = {32,2};
        struct GNU_DEF St_CANID_srP_Msg1 {
            u8 Reserve_1 : 7;
            u8 LongAccVd : 1;
            u16 VehSpd : 15;
            u8 Reserve_2 : 1;
            u8 TankPres : 8;
            u8 VehSpdVd : 2;
            u8 Reserve_3 : 3;
            u8 CrashInfo : 1;
            u16 Reserve_4 : 10;
            u16 LongACC : 10;
            u8 EPBSts : 3;
            u8 Reserve_5 : 1;
            u8 AYCItvInProgress : 1;
            u8 TCSItvInProgress : 1;
        };
        St_CANID_srP_Msg1 *pcan;
        pcan->VehSpd = GetSplitCanData((u64)*pbuff, VehSpd.LSB_bit, VehSpd.len);
        pcan->VehSpdVd = GetSplitCanData((u64)*pbuff, VehSpdVd.LSB_bit, VehSpdVd.len);
        if (g_timer.is_actived_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_srP_Msg1)].timid)) {
            g_timer.active_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_srP_Msg1)].timid,
                signal_time::sig_tm[SIG_CAST(u8, cid_srP_Msg1)].per);
        }
        {

            if (pcan->VehSpdVd == 0x0 || pcan->VehSpd > 0x14D5) {
                char *str = "---";
                set_property_aliase_value("speed_value", str);
            }
            else {
                float fkm_speed = pcan->VehSpd * VEHSPD_FACTOR;
                fkm_speed = (fkm_speed > VEHSPD_DISABLE_PHY) ? 0.f : fkm_speed;  /* rang judgment   */
                fkm_speed = (fkm_speed * VEHSPD_COEFF) + VEHSPD_DELTA;       /* v2 = v1 * coeff + delta */
                fkm_speed = (fkm_speed + 0.5) / 1.f;                         /* round */
                int km_speed = fkm_speed;
                vg_print("vechicle speed:%d\n", km_speed);
                char str_speed[50] = "";
                sprintf(str_speed, "%d", km_speed);
                set_property_aliase_value("text_speed_val", str_speed);
            }

        }
        return false;
    });
    mh.register_msg_handle(HU_CarSet1, [&](u8 *pbuff, int len) {
        const Singnal HU_HUD_FunctionSW = {24,1};
        const Singnal HU_HUD_DisplayModeSet = {25,2};
        const Singnal HU_HUD_SnowModeSet = {37,1};
        struct GNU_DEF St_CANID_HU_CarSet1 {
            u8 HeadLampHeightSet : 3;
            u8 NappingPatternSet : 1;
            u8 CentrollockingReq : 2;
            u8 MP5_HVACLinkageSts : 2;
            u8 DriverHeightSet : 8;
            u8 LightEmotionalAvailableUnlockSet : 2;
            u8 LightEmotionalAvailableLockSet : 3;
            u8 Reserve_1 : 3;
            u8 HU_HUD_FunctionSW : 1;
            u8 HU_HUD_DisplayModeSet : 2;
            u8 HU_HUD_ManualLightingAdjLevelSet : 5;
            u8 HU_HUD_ManualHeightAdjLevelSet : 5;
            u8 HU_HUD_SnowModeSet : 1;
            u8 HU_HUD_LanguageSet : 2;
            u32 Reserve_2 : 24;
        };
        St_CANID_HU_CarSet1 *pcan;
        pcan->HU_HUD_FunctionSW = GetSplitCanData((u64)*pbuff, HU_HUD_FunctionSW.LSB_bit, HU_HUD_FunctionSW.len);
        pcan->HU_HUD_DisplayModeSet = GetSplitCanData((u64)*pbuff, HU_HUD_DisplayModeSet.LSB_bit, HU_HUD_DisplayModeSet.len);
        pcan->HU_HUD_SnowModeSet = GetSplitCanData((u64)*pbuff, HU_HUD_SnowModeSet.LSB_bit, HU_HUD_SnowModeSet.len);
        if (g_timer.is_actived_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_HU_CarSet1)].timid)) {
            g_timer.active_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_HU_CarSet1)].timid,
                signal_time::sig_tm[SIG_CAST(u8, cid_HU_CarSet1)].per);
        }
        {
            bool sw = pcan->HU_HUD_FunctionSW;
            set_property_aliase_value_T("show_screen", sw);
        }
        {


        }

        return false;
    });
    mh.register_msg_handle(HUD_Status, [&](u8 *pbuff, int len) {
        static s8 bri_last = 0;
        static s8 ht_last = 0;
        const Singnal HUD_ManualLightingAdjLevel = {0,5};
        const Singnal HUD_ManualHeightAdjLevel = {8,5};
        struct GNU_DEF St_CANID_HUD_Status {
            u8 HUD_ManualLightingAdjLevel : 5;
            u8 HUD_SystemSt : 1;
            u8 HUD_DisplayModeSt : 2;
            u8 HUD_ManualHeightAdjLevel : 5;
            u8 HUD_SnowModeSt : 1;
            u8 HUD_LanguageSt : 2;
            u64 Reserve_1 : 48;
        };
        St_CANID_HUD_Status *pcan;
        pcan->HUD_ManualLightingAdjLevel = GetSplitCanData((u64)*pbuff, HUD_ManualLightingAdjLevel.LSB_bit, HUD_ManualLightingAdjLevel.len);
        pcan->HUD_ManualHeightAdjLevel = GetSplitCanData((u64)*pbuff, HUD_ManualHeightAdjLevel.LSB_bit, HUD_ManualHeightAdjLevel.len);
        if (g_timer.is_actived_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_HUD_Status)].timid)) {
            g_timer.active_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_HUD_Status)].timid,
                signal_time::sig_tm[SIG_CAST(u8, cid_HUD_Status)].per);
        }

        auto val = [](u8 index, u8 size, const signed char *tab, s8 *last) {
            s8 val;
            if (index < size) {
                val = tab[index];
                *last = val;
            }
            else {
                val = *last;
            }
            return val;
        };
        char val_str[50] = "";
        {
            s8 bri = val(pcan->HUD_ManualLightingAdjLevel, (u8)BRI_LEL_NUM, adj::bri_lv, &bri_last);
            sprintf(val_str, "%d", bri);
            set_property_aliase_value("bri_val", val_str);
        }
        {
            s8 ht = val(pcan->HUD_ManualHeightAdjLevel, (u8)HT_LEL_NUM, adj::ht_lv, &ht_last);
            sprintf(val_str, "%d", ht);
            set_property_aliase_value("ht_val", val_str);
        }

        return false;
    });
    mh.register_msg_handle(srE_Msg1, [&](u8 *pbuff, int len) {
        const Singnal JP_ACC_ErrorSts = {2,1};
        const Singnal ADS_ACCMode_ESP = {4,3};
        struct GNU_DEF St_srE_Msg1 {
            u8 JP_LatCtrl_ErrorSt : 2;
            u8 JP_ACC_ErrorSts : 1;
            u8 JP_AEB_ErrorSts : 1;
            u8 ADS_ACCMode_ESP : 3;
            u32 Reserve_1 : 20;
            u8 AEBDecCtrlRequest : 1;
            u8 AWBRequest : 1;
            u32 Reserve_2 : 19;
            u8 srE_Msg1Counter0 : 4;
            u8 Reserve_3 : 4;
            u8 srE_Msg1CheckSum0 : 8;
        };
        St_srE_Msg1 *pcan;
        pcan->JP_ACC_ErrorSts = GetSplitCanData((u64)*pbuff, JP_ACC_ErrorSts.LSB_bit, JP_ACC_ErrorSts.len);
        pcan->ADS_ACCMode_ESP = GetSplitCanData((u64)*pbuff, ADS_ACCMode_ESP.LSB_bit, ADS_ACCMode_ESP.len);
        if (icon::accspd_visible ||
            g_timer.is_actived_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_srE_Msg1)].timid)) {

            g_timer.active_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_srE_Msg1)].timid,
                signal_time::sig_tm[SIG_CAST(u8, cid_srE_Msg1)].per);
        }
        {
            icon::jdg_iconacc_col_gb(pcan->ADS_ACCMode_ESP);
            icon::jdg_iconacc_col_y((bool)pcan->JP_ACC_ErrorSts);
        }
        return false;
    });
    mh.register_msg_handle(mrADASCAN_F04, [&](u8 *pbuff, int len) {
        const Singnal ACC_Set_Speed = {32,8};
        struct GNU_DEF St_ADASCAN_F04 {
            u8 FCW_State : 3;
            u8 FCW_Warning_Level : 2;
            u8 FCW_ErrorSts : 1;
            u8 LDW_Warning : 2;
            u8 LDW_State : 3;
            u8 ACC_SCC_in_out_Remind : 3;
            u8 LDW_ErrorSts : 1;
            u8 IHC_Mainbeam_Request : 1;
            u8 IHC_State : 3;
            u8 ACC_SCC_NotEnable_Reason : 4;
            u8 BCM_HazardLight_Req : 1;
            u8 ACC_Driver_Cancel : 1;
            u8 ACC_CIPV_Indicator : 1;
            u8 ACC_Override_Indicator : 1;
            u8 ACC_Takeover_Indicator : 1;
            u8 ACC_Go_Indicator : 1;
            u8 ACC_Set_Headway : 3;
            u8 ACC_Set_Speed : 8;
            u8 RESERVE : 1;
            u8 ACC_Real_Headway : 3;
            u8 ADU_ErrorSts : 1;
            u8 LCC_Takeover_Request : 1;
            u8 LCC_ErrorSts : 1;
            u8 ADU_Counter4 : 4;
            u8 LKA_ErrorSts : 1;
            u8 IHC_ErrorSts : 1;
            u8 LCC_Line_Detect_Status : 2;
            u8 ADU_CheckSum4 : 8;
        };
        St_ADASCAN_F04 *pcan;
        pcan->ACC_Set_Speed = GetSplitCanData((u64)*pbuff, ACC_Set_Speed.LSB_bit, ACC_Set_Speed.len);
        if (icon::accspd_visible ||
            g_timer.is_actived_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_mrADASCAN_F04)].timid)) {

            g_timer.active_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_mrADASCAN_F04)].timid,
                signal_time::sig_tm[SIG_CAST(u8, cid_mrADASCAN_F04)].per);
        }
        {
            /* acc speed */
            icon::show_accspd(pcan->ACC_Set_Speed);
            /* acc speed end */
        }
        return false;
    });
    mh.register_msg_handle(mrADASCAN_F05, [&](u8 *pbuff, int len) {
        const Singnal ACC_State = {52,3};
        struct GNU_DEF St_mrADASCAN_F05 {
            u8 TSR_TarSpdReq : 8;
            u8 LKA_State : 3;
            u8 TSR_SWSetResp : 1;
            u8 TSR_ErrorSts : 1;
            u8 TSR_State : 3;
            u8 Cruise_mode_SWSetResp : 2;
            u8 NOP_VoicePromptSetResp : 2;
            u8 LKA_ModeResp : 2;
            u8 Cruise_DCLC_SWSetResp : 1;
            u8 LKA_Intervention : 2;
            u8 BVRW_SWSetResp : 1;
            u8 LDWSensiSetResp : 2;
            u8 NOP_SWSetResp : 1;
            u8 AES_SWSetResp : 1;
            u8 IHC_SWSetResp : 1;
            u8 AEB_SWSetResp : 1;
            u8 LSL_SWSetResp : 1;
            u8 LBS_SWSetResp : 2;
            u8 ETP_SWSetResp : 1;
            u8 DCLC_Req : 4;
            u8 LCC_State : 3;
            u8 FCW_SWSetResp : 1;
            u8 FCWSensiSetResp : 2;
            u8 ACC_State : 3;
            u8 AEB_State : 3;
            u8 ADU_CheckSum5 : 8;
            u8 ADU_Counter5 : 4;
        };
        St_mrADASCAN_F05 *pcan;
        pcan->ACC_State = GetSplitCanData((u64)*pbuff, ACC_State.LSB_bit, ACC_State.len);
        if (icon::accspd_visible ||
            g_timer.is_actived_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_mrADASCAN_F05)].timid)) {

            g_timer.active_timer_ex(signal_time::sig_tm[SIG_CAST(u8, cid_mrADASCAN_F05)].timid,
                signal_time::sig_tm[SIG_CAST(u8, cid_mrADASCAN_F05)].per);
        }
        {
            icon::jdg_iconacc_col_gb(pcan->ACC_State);
        }
        return false;
    });
    mh.register_msg_handle(srTCU_GearInfo, [&](u8 *pbuff, int len) {
        u8 igear;
        char *str_gear[5] = {" ","P","R","N","D"};
        if (hud::car_type == hud::car_t::CAR_8AT) {
            const Singnal GearPsn = {44,4};
            struct GNU_DEF St_srTCU_GearInfo {
                u8 DrvModDsp : 4;
                u8 DriveModeSts : 3;
                u8 Reserve_1 : 1;
                u8 ShiftRecommendation : 2;
                u8 Reserve_2 : 2;
                u8 CurrentGear : 4;
                u8 CurrentGearVd : 1;
                u8 TCU_ShiftTipInfo : 4;
                u8 Reserve_3 : 2;
                u8 TCUFeedback : 1;
                u8 IGCState : 1;
                u8 OilPressureWarn : 1;
                u8 Reserve_4 : 4;
                u8 TsmFailure : 1;
                u16 Reserve_5 : 9;
                u8 TsmID : 3;
                u8 Reserve_6 : 1;
                u8 GearPsn : 4;
                u8 srTCU_GearInfoCounter : 4;
                u8 EgdGearDsp : 4;
                u8 srTCU_GearInfoCheckSum : 8;
            };
            St_srTCU_GearInfo *pcan;
            pcan->GearPsn = GetSplitCanData((u64)*pbuff, GearPsn.LSB_bit, GearPsn.len);
            igear = pcan->GearPsn;
        }
        else {
            const Singnal CurrentGear = {0,3};
            struct GNU_DEF St_srHCAN_Msg1 {
                u8 CurrentGear : 3;
                u8 Reserve_1 : 1;
                u8 DriveModeSts : 3;
                u8 HVCUFeedback : 1;
                u8 VCUEVModeSts : 4;
                u16 GasPdlPsnRaw : 12;
                u8 ReadyLampSts : 1;
                u8 OilPressure : 1;
                u8 CurrentEVModeSts : 3;
                u8 Reserve_2 : 1;
                u8 TCU_FaultLamp : 1;
                u8 Reserve_3 : 7;
                u8 DisplayEVModeSts : 2;
                u8 Reserve_4 : 4;
                u8 GearPsn : 4;
                u8 CruVehSpdVd : 1;
                u8 CruSysSet : 1;
                u8 CruSysOn : 1;
                u8 BrkPdlPsnNC : 1;
                u8 BrkPdlPsnNO : 1;
                u8 TsmTempWarning : 2;
                u16 CruVehSpd : 9;
            };
            St_srHCAN_Msg1 *pcan;
            pcan->CurrentGear = GetSplitCanData((u64)*pbuff, CurrentGear.LSB_bit, CurrentGear.len);
            igear = pcan->CurrentGear;
        }
        return false;
    });
}
//XXX: end of register

// XXX: before update
void before_update() {
    //set_property_aliase_value_T("show_s_lane", lane_visible);
    //set_property_aliase_value_T("show_lane", lane_visible);
    set_property_aliase_value_T("show_dis", dis_visble);
    //set_property_aliase_value_T("show_ar_road", road_visible);
    icon::show_icon();
    debug_cnt++;
    debug_cnt %= debug_cirlce;
}