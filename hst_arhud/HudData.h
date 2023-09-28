#ifndef __HUD_DATA__H_
#define __HUD_DATA__H_


#define HUDSERVICE_SERVER_NAME "HudServiceMoudle"

enum
{
    HUDSERVICE_MCU_INFO = 0X100,                        //CAN 透传ID

    HUDSERVICE_IVINAVI_INFO = 0X101,                        //导航信息 透传ID
     /********************导航信息子ID start*****************/
    /*
    GPS 状态：0x00:GPS未定位，0x01:GPS已定位
    */
    HUDSERVICE_GPS_INFO = 0X10,                        //GPS状态

    /*
    下个路口名称
    */
    HUDSERVICE_NEXT_INTERSECTION_NAME_INFO = 0X12,     //下个路口名称

    /*
    下一个转向图标
    说明：
    在导航向导模式下，下一个转向信息更新时，发送此消息。默认转向信息是“0”，即没信息。
    0:   没信息  
    1-n：请参见文档【仪表中导航提示图标定义报告】
    */
    HUDSERVICE_NEXT_ICON_INFO = 0X14,                  //下一个转向图标

    /*
    离下一个路口距离 
    说明：离下个路口的距离信息。单位已经转换好了，直接显示字符串即可
    */
    HUDSERVICE_NEXT_CROSS_DISTANCE_INFO = 0X15,        //离下一个路口距离 

    /*
    离目的地距离
    */
    HUDSERVICE_TARGET_DISTANCE_INFO = 0X16,            //离目的地距离

    /*
    离目的地时间
    */
    HUDSERVICE_TARGET_TIME_INFO = 0X17,                //离目的地时间

    /*
    导航画面开始/结束
    1:导航开始； 0:导航关闭
    */
    HUDSERVICE_NAVIGATION_STATE_INFO = 0X1A,           //导航画面开始/结束

    /*
    当前定位省份信息
    */
    HUDSERVICE_NOW_PROVINCE_INFO = 0X1B,               //当前定位省份信息

    /*
    当前定位市信息
    */
    HUDSERVICE_NOW_CITY_INFO = 0X1C,                   //当前定位市信息

    /*
    当前定位区信息
    */
    HUDSERVICE_NOW_AREA_INFO = 0X1D,                   //当前定位区信息

    /********************导航信息子ID end*****************/


    E_NID_HUD_VEHICLE_SPEED_MSG_ID    = 0x64,   //stVehicleSpeed

    E_NID_HUD_WHEEL_ANGLE_MSG_ID      = 0x65,   //stWheelAngle

    E_NID_HUD_LAMP_T_LEFT_MSG_ID      = 0x66,   //stLampTurnLeft

    E_NID_HUD_LAMP_T_RIGHT_MSG_ID     = 0x67,   //stLampTurnRight

    E_NID_HUD_LAMP_T_POST_MSG_ID      = 0x68,   //stLampPosition
    
    E_NID_HUD_GEAR_DISPLAY_MSG_ID     = 0x69,   //stGearDisplay

};



struct CanInfo {
    uint32_t ID;             //CAN ID
    char info[8];       //CAN 数据
    bool timeOutFlag;   //超时标志 0 正常， 1 超时
}; 



struct NaviInfo {
    uint32_t naviID;      //Navi ID
    char info[512];       //CAN 数据 一般来说512个字节应该够了
}; 

/*
* IPK_vDisplay
*/
typedef struct __st_vs_
{
    bool       bTimeOutFlg;    //超时标志 0 正常， 1 超时
    uint16_t   iVehicleSpeed;  
}stVehicleSpeed;

/*
* SAS_SteerWheelAngle
*/
typedef struct __st_wa_
{
    bool       bTimeOutFlg;    //超时标志 0 正常， 1 超时
    uint16_t   iWheelAngle;
}stWheelAngle;

/*
* BCM_TurnIndicatorLeft / BCM_TurnIndicatorRight / BCM_PositionLightSts
*/
typedef struct __st_tl_
{
    bool       bTimeOutFlg;    //超时标志 0 正常， 1 超时
    uint8_t    iLampStatus;
}stLampStatus;

/*
* VCU_ModeGearDisplaySts
*/
typedef struct __st_gd_
{
    bool       bTimeOutFlg;    //超时标志 0 正常， 1 超时
    uint8_t    iGearDisp;
}stGearDisplay;








#endif
