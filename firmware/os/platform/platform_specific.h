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

#if defined(PLATFORM_LAMBDA_REV0)
/*
    Lambda motherboard, revision 0
*/
#include <platform/lambda_rev0/lambda.h>

#else
/*
    Error: Undefined/unknown target platform.  Maybe add a section to this file to support your
    platform?
*/
#error No platform-specific include file for the target architecture
#endif


#endif
