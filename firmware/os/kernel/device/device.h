#ifndef KERNEL_DEVICE_DEVICE_H_INC
#define KERNEL_DEVICE_DEVICE_H_INC
/*
	device.h: declarations of functions and types relating to the hardware abstraction layer

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th February 2012.
*/

#include <kernel/device/devctl.h>
#include <kernel/include/types.h>
#include <klibc/include/errno.h>


#define DEVICE_NAME_LEN			(8)		/* Max length of device name, including terminating \0 	*/
#define DEVICE_MAX_SUBDEVICES	(61)	/* e.g. ataa1, ataa2, ataa3, ...ataaZ                   */


/*
    Device states
*/
typedef enum
{
    DEV_STATE_UNKNOWN           = 0,
    DEV_STATE_READY             = 1,
    DEV_STATE_OFFLINE           = 2
} dev_state_t;


/*
    High-level device types
*/
typedef enum
{
    DEV_TYPE_NONE               = 0x00,
	DEV_TYPE_BLOCK              = 0x01,
	DEV_TYPE_CHARACTER          = 0x02,
	DEV_TYPE_NET                = 0x03,
	DEV_TYPE_SERIAL			    = 0x04,
	DEV_TYPE_RTC                = 0x05,
	DEV_TYPE_MEM                = 0x06,
	DEV_TYPE_NVRAM              = 0x07,
	DEV_TYPE_TIMER              = 0x08,
	DEV_TYPE_MULTI              = 0x09
} dev_type_t;


/*
    Device subtypes
*/
typedef enum
{
    DEV_SUBTYPE_NONE            = 0x00,
    DEV_SUBTYPE_MASS_STORAGE    = 0x01,
    DEV_SUBTYPE_PARTITION       = 0x02,
    DEV_SUBTYPE_ETHERNET        = 0x03,
    DEV_SUBTYPE_PS2PORT         = 0x04
} dev_subtype_t;


/*
    The main device structure, dev_t
*/
struct dev;
typedef struct dev dev_t;

struct dev
{
    dev_type_t      type;
    dev_subtype_t   subtype;
    dev_state_t     state;
    char            name[DEVICE_NAME_LEN];
    const char *    human_name;
    void *          data;
    u32             irql;
    void *          base_addr;
    u32             mem_len;

    u32             block_size;
    u32             len;

    s32             (*control)(dev_t *dev, const devctl_fn_t fn, const void *in, void *out);
    s32             (*read)(dev_t *dev, ku32 offset, u32 *len, void *buf);
    s32             (*write)(dev_t *dev, ku32 offset, u32 *len, const void *buf);
    s16             (*getc)(dev_t *dev);
    s32             (*putc)(dev_t *dev, const char c);
    s32             (*shut_down)(dev_t *dev);

    dev_t *         parent;
    dev_t *         first_child;
    dev_t *         prev_sibling;
    dev_t *         next_sibling;
};

typedef s32 (*driver_init_fn)(dev_t *);


/*
    Function declarations
*/
s32 dev_init();
s32 dev_enumerate();
dev_t *dev_get_root();
s32 dev_add_child(dev_t *parent, dev_t *child);

s32 dev_create(const dev_type_t type, const dev_subtype_t subtype, const char * const dev_name,
                 ku32 irql, void *base_addr, dev_t **dev, const char * const human_name,
                 dev_t *parent_dev, s32 (*init_fn)(dev_t *));

s32 dev_destroy(dev_t *dev);
char dev_get_type_char(const dev_t * const dev);

dev_t *dev_find(const char * const name);
dev_t *dev_find_subtree(const char * const name, dev_t *node);
dev_t *dev_get_next(dev_t *node);

s32 dev_add_suffix(char * const name);

s32 dev_read_unimplemented(dev_t * const dev, ku32 offset, u32 *len, void *buf);
s32 dev_write_unimplemented(dev_t * const dev, ku32 offset, u32 *len, const void *buf);
s32 dev_control_unimplemented(dev_t * const dev, const devctl_fn_t fn, const void *in, void *out);
s16 dev_getc_unimplemented(dev_t *dev);
s32 dev_putc_unimplemented(dev_t *dev, const char c);
s32 dev_shut_down_unimplemented(dev_t * const dev);

#endif

