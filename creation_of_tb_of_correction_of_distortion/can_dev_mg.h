#pragma once
#include <functional>
#include <ControlCAN.h>
namespace can_devs {

  using can_handle = std::function<void(VCI_CAN_OBJ*)>;
  void reg_can_handle(UINT can_id, can_handle can_h);
  const int can_channels = 2;
  const int max_device_count = 50;
  enum baud_def {
    bps_10k,
    bps_20k,
    bps_40k,
    bps_50k,
    bps_80k,
    bps_100k,
    bps_125k,
    bps_200k,
    bps_250k,
    bps_400k,
    bps_500k,
    bps_666k,
    bps_800k,
    bps_1000k,
    bps_33p33k,
    bps_66p66k,
    bps_83p33k,
    bps_cnt
  };
  struct {
    const char* name;
    int timing0, timing1;
  } baud_items[bps_cnt] = {
    {"10 Kpps",0x31,0x1c},
    {"20 Kpps",0x18,0x1c},
    {"40 Kbps",0x87,0xff},
    {"50 Kbps",0x09,0x1c},
    {"80 Kbps",0x83,0xff},
    {"100 Kbps",0x04,0x1c},
    {"125 Kbps",0x03,0x1c},
    {"200 Kbps",0x81,0xfa},
    {"250 Kbps",0x01,0x1c},
    {"400 Kbps",0x80,0xb6},
    {"500 Kbps",0x00,0x1c},
    {"666 Kbps",0x80,0xb6},
    {"800 Kbps",0x00,0x16},
    {"1000 Kbps",0x00,0x14},
    {"33.33 Kbps",0x09,0x6f},
    {"66.66 Kbps",0x04,0x6f},
    {"83.33 Kbps",0x03,0x6f},
  };
  struct base_can{
    int _dev_type{ VCI_USBCAN1 };//or VCU_USBCAN2
    int _channel_cnt{ 1 };
    int baud_indeics[can_channels] = { 0,0 };
  };
  extern base_can devices_states[max_device_count];
  /**
   * @brief The process of enumerating all CAN devices plugged in the computer 
   * @param state  1 devices plugged,0 devices unplugged
  */
  void can_devices(int state);
  /**
   * @brief Connect a CAN device
   * @param dev_index Index of CAN devices connected to the computer
   * @return 
  */
  int open_device(int dev_index);

  int close_device(int dev_index);
  /**
   * @brief the process of receiving messages which included in can objects
  */
  void receive_msg();
  /**
   * @brief Send CAN objects to CAN device 
   * @param dev_index Index of CAN devices connected to the computer
   * @param channel Channel of CAN device
   * @param pcan_obj CAN object which contain message planed transfer to target device
   * @param count  Count of CAN object planed to send.must less than 1000
   * @return Count of CAN object sended actually
  */
  int send_message(int dev_index, int channel, VCI_CAN_OBJ* pcan_obj, int count);
  /**
   * @brief Count of valid CAN devices in the computer
  */
  extern int device_num;
}