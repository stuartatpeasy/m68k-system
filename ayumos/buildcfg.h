#ifndef BUILDCFG_H_INC
#define BUILDCFG_H_INC
/*
	buildcfg.h: configure a build for a particular platform and target, and set options.

	Part of ayumos


	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
*/


#define KMALLOC_HEAP

#define DEBUG_KMALLOC
#define DEBUG_KSYM

/* Main build options */
#define WITH_NETWORKING

/* Driver build options */
#define WITH_DRV_ENCX24J600
#define WITH_DRV_PS2CONTROLLER

#define IPV4_VERIFY_CHECKSUM 1      /* Verify checksum on incoming IPv4 packets                 */
#define ICMP_VERIFY_CHECKSUM 1      /* Verify checksum on incoming ICMP packets                 */


/* FIXME - target arch should be defined in platform/platform_specific.h, not here */
#define TARGET_MC68010

#define PLATFORM_LAMBDA
#define PLATFORM_REV    1

#endif

