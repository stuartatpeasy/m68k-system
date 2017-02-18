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
#define WITH_FS_EXT2
#define WITH_FS_FAT
#define WITH_FS_ROMFS
#define WITH_KEYBOARD
#define WITH_MASS_STORAGE
#define WITH_RTC

/* Driver build options */
#define WITH_DRV_HID_PS2CONTROLLER
#define WITH_DRV_MST_ATA
#define WITH_DRV_MST_PARTITION
#define WITH_DRV_NET_ENCX24J600
#define WITH_DRV_RTC_DS17485
#define WITH_DRV_SER_MC68681

/* Networking options */
#define WITH_NETWORKING
#define WITH_NET_ARP
#define WITH_NET_ETHERNET
#define WITH_NET_ICMP
#define WITH_NET_IPV4
#define WITH_NET_TCP
#define WITH_NET_UDP

/* Network protocol-specific options */
#define IPV4_VERIFY_CHECKSUM 1      /* Verify checksum on incoming IPv4 packets                 */
#define ICMP_VERIFY_CHECKSUM 1      /* Verify checksum on incoming ICMP packets                 */

/* Memory layout options */
#define SLAB_RESERVED_MEM   65536   /* Memory reserved for slabs                                */

/* FIXME - target arch should be defined in platform/platform_specific.h, not here */
#define TARGET_MC68010
#define TARGET_BIGENDIAN

#define PLATFORM_LAMBDA
#define PLATFORM_REV    1

#endif

