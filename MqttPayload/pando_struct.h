/*******************************************************
 * File name: pando_struct.h
 * Author: Zhao Wenwu
 * Versions: 1.0
 * Description: provide necessary struct for developers
 * History:
 *   1.Date:
 *     Author:
 *     Modification:    
 *********************************************************/


#ifndef PANDP_PROTOCOL_TOOL_H
#define PANDP_PROTOCOL_TOOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "platform_functions.h"

//Params of payload in data, event and command
struct params_block
{
    uint16_t count;
    //struct TLV tlv[];
};

#ifdef __cplusplus
}
#endif
#endif //PANDP_PROTOCOL_TOOL_H


