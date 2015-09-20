#ifndef DEVICE_DEVICE_H_INC
#define DEVICE_DEVICE_H_INC
/*
	device.h: declarations of functions and types relating to the hardware abstraction layer

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include "device/devctl.h"
#include "klibc/errno.h"
#include "include/types.h"

#define DEVICE_NAME_LEN			(6)		/* Max length of device name, including terminating \0 	*/
#define MAX_DEVICES             (64)    /* Maximum number of devices allowed by the system      */

#define DEVICE_MAX_SUBDEVICES	(61)	/* e.g. ataa1, ataa2, ataa3, ...ataaZ */

#define INVALID_DEVICE_ID		(-1)	/* Device guaranteed not to exist */


enum device_type
{
	DEVICE_TYPE_BLOCK       = 0x01,
	DEVICE_TYPE_CHARACTER   = 0x02,
	DEVICE_TYPE_NET         = 0x03
};

typedef enum device_type device_type_t;

enum device_class
{
    DEVICE_CLASS_DISC       = 0x01,
    DEVICE_CLASS_PARTITION  = 0x02
};

typedef enum device_class device_class_t;

struct device_driver
{
	const char *name;
	u32 version;

	s32 (*init)();
	s32 (*shut_down)();

	s32 (*read)(void *data, ku32 offset, ku32 len, void *buf);
	s32 (*write)(void *data, ku32 offset, ku32 len, const void *buf);

	s32 (*control)(void *data, ku32 function, void *in, void *out);
};

typedef struct device_driver device_driver_t;


struct device
{
	enum device_type type;
	enum device_class class;
	struct device_driver *driver;
	char name[DEVICE_NAME_LEN];
	void *data;
};

typedef struct device device_t;

/*
    Variable declarations
*/
const char * const g_device_sub_names;
device_t * g_devices[MAX_DEVICES];


/*
    Function declarations
*/
u32 driver_init();
void driver_shut_down();
s32 create_device(const device_type_t type, const device_class_t class,
                         struct device_driver * const driver, const char *name, void * const data);

s32 driver_method_not_implemented();

device_t *get_device_by_name(const char * const name);

s32 device_read(const device_t * const dev, ku32 offset, u32 len, void *buf);
s32 device_write(const device_t * const dev, ku32 offset, u32 len, const void *buf);

s32 device_control(const device_t * const dev, ku32 function, void *in, void *out);


#endif

