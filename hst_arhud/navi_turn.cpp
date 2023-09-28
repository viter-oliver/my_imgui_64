#include "navi_turn.h"
/**
  4     5
   \   /
    \ /
2<------->3
   / | |  \
  /  | |   \
 6   8 19   7
 */
float prerotate_angle(int navi_id,float& ori_x){
  const float PI=3.1415926545f;
  const struct 
  {
    float ori_x;
    float rotate_angle;
  } a_p[35]={
    0,0,//0     
    0,0,//1
    -1.5,PI*0.5,//2 <-
    1.5,PI*0.5,//3 ->
    -1.5,PI*0.25,//4 left up
    1.5,PI*0.25,//5 right up
    -1.5,PI*0.75,//6 left dn
    1.5,PI*0.75,//7 left rt
    -1.5,PI,//8
    0,0,//9
    0,0,//10
    0,0,//11
    0,0,//12
    0,0,//13
    0,0,//14
    0,0,//15
    0,0,//16
    0,0,//17
    0,0,//18
    1.5,PI,//19 right rtn
    0,0,//20
    0,0,//21
    0,0,//22
    0,0,//23
    0,0,//24
    0,0,//25
    0,0,//26
    0,0,//27
    0,0,//28
    0,0,//29
    0,0,//30
    0,0,//31
    0,0,//32
    0,0,//33
    0,0,//34
  };
  ori_x=a_p[navi_id].ori_x;
  return a_p[navi_id].rotate_angle;
}