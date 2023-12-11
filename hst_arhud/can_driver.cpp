#include "can_driver.h"
#include "msg_host.h"
using namespace msg_utility;
u8 const CanPosMapTable[64] = {
       7, 6, 5, 4, 3, 2, 1, 0,
       15, 14, 13, 12, 11, 10, 9, 8,
       23, 22, 21, 20, 19, 18, 17, 16,
       31, 30, 29, 28, 27, 26, 25, 24,
       39, 38, 37, 36, 35, 34, 33, 32,
       47, 46, 45, 44, 43, 42, 41, 40,
       55, 54, 53, 52, 51, 50, 49, 48,
       63, 62, 61, 60, 59, 58, 57, 56
};

/**
 * 根据lsb和长度获取对应的CAN数据
 */
u32 GetSplitCanData(u64 data, u8 LSB, u8 len) {
    u8 RealPos = 0;
#ifdef MOTO_LSB_ORDER_ENCODE
    RealPos = 62 + len - CanPosMapTable[LSB];
#elif defined(MOTO_MSB_ORDER_ENCODE)
    RealPos = 63 - CanPosMapTable[LSB];
#elif defined(INTEL_ORDER_ENCODE)
    RealPos = LSB + len - 1;
    Reverse_u64_byte(&data);
#endif
    u32 can_data = GET_N_BIT(data, RealPos, len);
    return can_data;
}

/**
 * 根据lsb和长度设置对应的CAN数据
 */
void SetSplitCanData(u64 *pData, u8 LSB, u8 len, u32 val) {
    u8 RealPos = 0;
#ifdef MOTO_LSB_ORDER_ENCODE
    RealPos = 62 + len - CanPosMapTable[LSB];
#elif defined(MOTO_MSB_ORDER_ENCODE)
    RealPos = 63 - CanPosMapTable[LSB];
#elif defined(INTEL_ORDER_ENCODE)
    RealPos = LSB + len - 1;
    Reverse_u64_byte(pData);
#endif
    CLEAR_N_BIT(*pData, RealPos, len);
    u64 tempVal = val;
    *pData |= (tempVal << (RealPos - len + 1));

#ifdef INTEL_ORDER_ENCODE
    Reverse_u64_byte(pData);
#endif
}

