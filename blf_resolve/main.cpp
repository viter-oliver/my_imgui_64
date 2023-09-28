#include <stdio.h>
#include <string>
#include <iostream>
#include "binlog.h"
using namespace std;
int main(int argc,char** argv){
  if(argc<2){
    printf("input blf file please!\n");
  }
  
  VBLObjectHeaderBase base;
  VBLCANMessage message;
  VBLCANMessage2_t message2;
  VBLEnvironmentVariable variable;
  VBLEthernetFrame ethframe;
  VBLAppText appText;
  VBLCANDriverErrorExt can_erro;
  VBLFileStatisticsEx statistics = { sizeof(statistics) };
  BOOL bSuccess;
  HANDLE hFile=BLCreateFile(argv[1],GENERIC_READ);
  if(INVALID_HANDLE_VALUE==hFile){
    return 0;
  }
  BLGetFileStatisticsEx(hFile, &statistics);
  auto& st=statistics.mMeasurementStartTime;
  printf("date: %d-%d-%d-%d-%d-%d\n",st.wMonth,st.wDayOfWeek,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
  while(BLPeekObject(hFile,&base)){
    printf("obj_type:%d\n", base.mObjectType);
    switch(base.mObjectType){
      case BL_OBJ_TYPE_CAN_MESSAGE:
      case BL_OBJ_TYPE_CAN_MESSAGE2:
      message2.mHeader.mBase=base;
      if(BLReadObjectSecure(hFile,&message2.mHeader.mBase,sizeof(message2))){
        auto& data=message2.mData;
        printf("id:0x%x dlc:0x%x data[%x %x %x %x %x %x %x %x]\n",message2.mID,message2.mDLC,data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]),
        BLFreeObject(hFile, &message2.mHeader.mBase);
      }
      break;
      case BL_OBJ_TYPE_CAN_DRIVER_ERROR:
        can_erro.mHeader.mBase = base;
        if (BLReadObjectSecure(hFile, &can_erro.mHeader.mBase, sizeof(can_erro))) {
          printf("can error:%d\n", can_erro.mErrorCode);
          BLFreeObject(hFile, &can_erro.mHeader.mBase);
        }
        break;
      case BL_OBJ_TYPE_ENV_INTEGER:
      case BL_OBJ_TYPE_ENV_DOUBLE:
      case BL_OBJ_TYPE_ENV_STRING:
      case BL_OBJ_TYPE_ENV_DATA:
       variable.mHeader.mBase = base;
      if(BLReadObjectSecure(hFile,&variable.mHeader.mBase,sizeof(variable))){
        printf("env[%s]\n",variable.mName);
        BLFreeObject(hFile, &variable.mHeader.mBase);
      }
      break;
      case BL_OBJ_TYPE_ETHERNET_FRAME:
      
      break;
      case BL_OBJ_TYPE_APP_TEXT:
        appText.mHeader.mBase = base;
        if (BLReadObjectSecure(hFile, &appText.mHeader.mBase, sizeof(appText))) {
          printf("%s\n", appText.mText);
          BLFreeObject(hFile, &appText.mHeader.mBase);
        }
      break;
      default:
        BLSkipObject(hFile, &base);
        break;
    }
  }
  BLCloseHandle(hFile);
  return 0;
}