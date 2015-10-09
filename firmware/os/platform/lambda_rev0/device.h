#ifndef BOARD_LAMBDA_DEVICE_H_INC
#define BOARD_LAMBDA_DEVICE_H_INC
/*
    Hardware device enumeration for the "lambda" (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <platform/lambda_rev0/device.h>

#include <device/device.h>
#include <include/defs.h>
#include <include/types.h>

s32 plat_dev_enumerate(dev_t *root_dev);

#endif
