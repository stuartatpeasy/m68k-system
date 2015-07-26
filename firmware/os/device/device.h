#ifndef __OS_DEVICE_DEVICE_H__
#define __OS_DEVICE_DEVICE_H__
/*
	device.h: declarations of functions and types relating to the hardware abstraction layer

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include "klibc/errno.h"
#include "include/types.h"

#define DEVICE_NAME_LEN			(6)		/* Max length of device name, including terminating \0 	*/
#define MAX_DEVICES             (64)    /* Maximum number of devices allowed by the system      */

#define DEVICE_MAX_SUBDEVICES	(61)	/* e.g. ataa1, ataa2, ataa3, ...ataaZ */

#define INVALID_DEVICE_ID		(-1)	/* Device guaranteed not to exist */


/* Return type for driver functions */
typedef enum
{
	DRIVER_OK = 0,
	DRIVER_NOT_IMPLEMENTED,		/* Function not implemented by this driver or device			*/
	DRIVER_INVALID_DEVICE,		/* No such device												*/
	DRIVER_TOO_MANY_DEVICES,	/* Cannot create any more devices								*/
	DRIVER_INVALID_SEEK,		/* Attempted to seek to an invalid/non-existent position		*/
	DRIVER_MEDIA_ERROR,
	DRIVER_NO_MEDIA,			/* No disc inserted, etc.										*/
	DRIVER_ABORT,				/* The device aborted the request								*/
	DRIVER_MEDIA_CHANGED,		/* The media was changed										*/
	DRIVER_WRITE_PROTECT,		/* The device is write-protected								*/
	DRIVER_DATA_ERROR,
	DRIVER_CRC_ERROR,			/* Checksum check failed										*/
	DRIVER_TIMEOUT,				/* The device failed to respond in a timely manner				*/

	DRIVER_UNKNOWN_ERROR		/* Unknown error.  Sad times									*/
} driver_ret;


enum device_type
{
	DEVICE_TYPE_BLOCK,
	DEVICE_TYPE_CHARACTER
};

typedef enum device_type device_type_t;

enum device_class
{
    DEVICE_CLASS_DISC,
    DEVICE_CLASS_PARTITION
};

typedef enum device_class device_class_t;

struct device_driver
{
	const char *name;
	u32 version;

	driver_ret (*init)();
	driver_ret (*shut_down)();

	driver_ret (*read)(void *data, ku32 offset, ku32 len, void *buf);
	driver_ret (*write)(void *data, ku32 offset, ku32 len, const void *buf);

	driver_ret (*control)(void *data, ku32 function, void *in, void *out);
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
driver_ret create_device(const device_type_t type, const device_class_t class,
                         struct device_driver * const driver, const char *name, void * const data);

u32 driver_method_not_implemented();

device_t *get_device_by_name(const char * const name);

driver_ret device_read(const device_t * const dev, ku32 offset, u32 len, u8 *buf);
driver_ret device_write(const device_t * const dev, ku32 offset, u32 len, ku8 *buf);

driver_ret device_control(const device_t * const dev, ku32 function, void *in, void *out);


#endif

