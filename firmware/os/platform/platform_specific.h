#ifndef PLATFORM_PLATFORM_SPECIFIC_H_INC
#define PLATFORM_PLATFORM_SPECIFIC_H_INC
/*
    This header #includes platform-specific headers, in case they e.g. define inline'd functions.

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#ifndef IN_PLATFORM_H
    #error "This file should not be #include'd directly."
#endif

#if !defined(PLATFORM_REV)
    #error "No platform revision (PLATFORM_REV) defined.  Check build conf in buildcfg.h"
#endif

/* Configuration for specific platforms */
#ifdef PLATFORM_LAMBDA
    /*
        "Lambda" motherboard, all revisions
    */

    /* Definitions which depend on hardware revision */
    #if (PLATFORM_REV == 0)
        #define BUG_ATA_BYTE_SWAP
        #define ATA_REG_OFFSET      (1)
    #elif (PLATFORM_REV == 1)
        #define ATA_REG_OFFSET      (0)
    #else
        #error "Unsupported lambda platform revision"
    #endif

    /* Definitions not dependent on hardware revision */
    #define ATA_REG_BASE        (0x20)
    #define ATA_ALT_REG_BASE    (0x10)
    #define ATA_REG_SHIFT       (1)

    #include <platform/lambda/lambda.h>
#else
    /*
        Error: Undefined/unknown target platform.  Maybe add a section to this file to support your
        platform?
    */
    #error "No/unsupported target platform specified.  Check build conf in buildcfg.h"
#endif

#endif
