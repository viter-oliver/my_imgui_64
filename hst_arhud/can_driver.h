#include "msg_host.h"
using namespace msg_utility;
#define MOTO_LSB_ORDER_ENCODE
#define GET_N_BIT(number, msb_pos, len) ((number >> (msb_pos - len + 1)) & (((u64)1 << len) - 1))
#define SET_N_BIT(number, msb_pos ,len, val) do{CLEAR_N_BIT(number, msb_pos ,len); number |= (val << (msb_pos+1-len));}while(0)
#define CLEAR_N_BIT(number, msb_pos, len) (number = (~((((u64)1<<len)-1)<<(msb_pos-len+1))) & number)

/**
 * 根据lsb和长度获取对应的CAN数据
 */
u32 GetSplitCanData(u64 data, u8 LSB, u8 len);

/**
 * 根据lsb和长度设置对应的CAN数据
 */
void SetSplitCanData(u64 *pData, u8 LSB, u8 len, u32 val);

struct Singnal {
    u8 LSB_bit;
    u8 len;
};
