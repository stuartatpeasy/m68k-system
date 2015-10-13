#ifndef DEVICE_DEVICE_H_INC
#define DEVICE_DEVICE_H_INC
/*
	device.h: declarations of functions and types relating to the hardware abstraction layer

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include "device/devctl.h"
#include "include/types.h"
#include "klibc/errno.h"


#define DEVICE_NAME_LEN			(8)		/* Max length of device name, including terminating \0 	*/
// FIXME REMOVE
#define MAX_DEVICES             (64)    /* Maximum number of devices allowed by the system      */

#define DEVICE_MAX_SUBDEVICES	(61)	/* e.g. ataa1, ataa2, ataa3, ...ataaZ */

#define INVALID_DEVICE_ID		(-1)	/* Device guaranteed not to exist */

#define IRQL_NONE               (0)     /* = "no interrupt generated by this device"            */


typedef enum
{
    DEV_STATE_UNKNOWN           = 0,
    DEV_STATE_READY             = 1,
    DEV_STATE_OFFLINE           = 2
} dev_state_t;


typedef enum
{
    DEV_TYPE_NONE           = 0x00,
	DEV_TYPE_BLOCK          = 0x01,
	DEV_TYPE_CHARACTER      = 0x02,
	DEV_TYPE_NET            = 0x03,
	DEV_TYPE_SERIAL			= 0x04,
	DEV_TYPE_RTC            = 0x05,
	DEV_TYPE_MULTI          = 0x06,
} dev_type_t;


typedef enum
{
    DEV_SUBTYPE_NONE            = 0x00,
    DEV_SUBTYPE_MASS_STORAGE    = 0x01,
    DEV_SUBTYPE_PARTITION       = 0x02
} dev_subtype_t;


struct dev;
typedef struct dev
{
    dev_type_t          type;
    dev_subtype_t       subtype;
    void *              driver;
    char                name[DEVICE_NAME_LEN];
    void *              data;
    u32                 irql;
    void *              base_addr;
    u32                 len;

    struct dev *        parent;
    struct dev *        first_child;
    struct dev *        prev_sibling;
    struct dev *        next_sibling;
    dev_state_t         state;
} dev_t;


// FIXME replace all these void* ptrs with dev_t * ?
typedef struct
{
	const char *name;
	u32 version;

	s32 (*init)();
	s32 (*shut_down)();

	s32 (*read)(void *data, ku32 offset, ku32 len, void *buf);
	s32 (*write)(void *data, ku32 offset, ku32 len, const void *buf);

	s32 (*control)(void *data, ku32 function, void *in, void *out);
} block_driver_t;


typedef struct
{
    s16 (*getc)(dev_t *dev);
    s16 (*putc)(dev_t *dev, const char c);
    s32 (*set_baud_rate)(dev_t *dev, ku32 baud_rate);
    ku32 (*get_baud_rate)(dev_t *dev);
} serial_ops_t;


typedef struct
{
    s32 (*get_time)(dev_t *dev, rtc_time_t *tm);
    s32 (*set_time)(dev_t *dev, const rtc_time_t *tm);
} rtc_ops_t;


/*
    Variable declarations
*/
const char * const g_device_sub_names;
dev_t * g_devices[MAX_DEVICES];      // FIXME REMOVE


/*
    Function declarations
*/
u32 driver_init();
void driver_shut_down();

s32 dev_enumerate();
dev_t *dev_get_root();
s32 dev_add_child(dev_t *parent, dev_t *child);

s32 dev_create(dev_type_t type, dev_subtype_t subtype, const char * const name, ku32 irql,
				void *base_addr, dev_t **dev);

dev_t *dev_find(const char * const name);
dev_t *dev_find_subtree(const char * const name, dev_t *node);

s32 dev_add_suffix(char * const name);

// FIXME REMOVE
s32 create_device(const dev_type_t type, const dev_subtype_t class, void * const driver,
                  const char *name, void * const data);

s32 driver_method_not_implemented();

dev_t *get_device_by_name(const char * const name);

s32 device_read(const dev_t * const dev, ku32 offset, u32 len, void *buf);
s32 device_write(const dev_t * const dev, ku32 offset, u32 len, const void *buf);

s32 device_control(const dev_t * const dev, ku32 function, void *in, void *out);


#endif

