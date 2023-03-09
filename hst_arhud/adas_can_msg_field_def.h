//
// Created by v_f_z on 2022/6/30.
//

#ifndef MY_APPLICATION_ADAS_CAN_MSG_FIELD_DEF_H
#define MY_APPLICATION_ADAS_CAN_MSG_FIELD_DEF_H
enum Vehicle_Type{no_vehicle,car,minibus,truck,specialcar};
enum TSR_SIGN_TYPE{
    regular10,
    regular20,
    regular30,
    regular40,
    regular50,
    regular60,
    regular70,
    regular80,
    regular90,
    regular100,
    regular110,
    regular120,
    regular130,
    regular140,
    regular5 = 100,
    regular15,
    regular25,
    regular35,
    regular45,
    regular55,
    regular65,
    regular75,
    regular85,
    regular95,
    regular105,
    regular115,
    regular125,
    regular135,
    regular145,
};
enum enSound_Type{
    Silent,
    LDW_Left,
    LDW_Right,
    HMW,
    TSR,
    UFCW,
    FCW_PCW,
    STOP_GO,
};
enum enHeadwayWarningLevel{
    no_recognize,
    HWM_BIGGER_THAN_HWConfig,
    HWM_LESS_EQUAL_THAN_HWConfig,
    HW_reserved
};
#endif //MY_APPLICATION_ADAS_CAN_MSG_FIELD_DEF_H
