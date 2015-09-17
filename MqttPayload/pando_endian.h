//  Copyright (c) 2015 Pando. All rights reserved.
//  PtotoBuf:   ProtocolBuffer.h
//
//  Create By ZhaoWenwu On 15/01/24.

#ifndef PANDO_ENDIAN_H
#define PANDO_ENDIAN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "platform_functions.h"

//大小端转换函数
uint16_t net16_to_host(uint16_t A);
uint32_t net32_to_host(uint32_t A);
uint64_t net64_to_host(uint64_t A);
float    net32f_to_host(float A);
double   net64f_to_host(double A);

#define host16_to_net  net16_to_host
#define host32_to_net  net32_to_host
#define host64_to_net  net64_to_host
#define host32f_to_net net32f_to_host
#define host64f_to_net net64f_to_host

#ifdef __cplusplus
}
#endif
#endif



