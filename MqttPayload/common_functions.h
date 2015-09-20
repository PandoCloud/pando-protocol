//  Copyright (c) 2014 Pando. All rights reserved.
//  Platform:   common_functions.h
//
//  Create By ZhaoWenwu On 15/05/17.


#ifndef PLATFORM_FUNCTIONS_H
#define PLATFORM_FUNCTIONS_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ESP8266_PLANTFORM

#else

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#endif

#include "pando_machine.h"

void show_package(uint8_t *buffer, uint16_t length);

#ifdef __cplusplus
}
#endif
#endif