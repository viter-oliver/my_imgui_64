//
// Created by v_f_z on 2022/6/29.
//

#ifndef MY_APPLICATION_ADAS_CAN_DEF_H
#define MY_APPLICATION_ADAS_CAN_DEF_H
#define AEB_WARNING 0x700
#define Vehicle_frame_a1 0x76d
#define Vehicle_frame_a2 0x76e
#define Vehicle_frame_c 0x76f

#define Vehicle_frame_b0 0x770
#define Vehicle_frame_b7 0x777
#define Pedestrian_frame_a 0x77a
#define Pedestrian_frame_bb 0x77b
#define Pedestrian_frame_bc 0x77c
#define Pedestrian_frame_bd 0x77d
#define Pedestrian_frame_c 0x77f

#define lane_frame_1_l 0x5f0
#define lane_frame_1_h 0x5f1
#define lane_frame_2_l 0x5f2
#define lane_frame_2_h 0x5f3
#define lane_frame_3_l 0x5f4
#define lane_frame_3_h 0x5f5
#define lane_frame_4_l 0x5f6
#define lane_frame_4_h 0x5f7
#define lane_frame_c 0x5f8

#define TSR_frame_A 0x761
#define TSR_frame_B1 0x762
#define TSR_frame_B2 0x763
#define TSR_frame_B3 0x764
#define TSR_frame_C 0x765

#define num_in_range(x,min,max) (unsigned)(x-min)<=(max-min)
#endif //MY_APPLICATION_ADAS_CAN_DEF_H
