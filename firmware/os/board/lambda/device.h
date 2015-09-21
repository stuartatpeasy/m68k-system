#ifndef BOARD_LAMBDA_DEVICE_H_INC
#define BOARD_LAMBDA_DEVICE_H_INC
/*
    Hardware device enumeration for the "lambda" (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <board/lambda/device.h>
#include <device/device.h>
#include <include/defs.h>
#include <include/types.h>

s32 b_dev_enumerate(dev_t *first_dev);

#endif
