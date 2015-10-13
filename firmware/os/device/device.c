/*
	device.c: device and device driver management

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <kutil/kutil.h>
#include <device/device.h>
#include <platform/platform.h>


/* Characters used to identify "sub-devices", e.g. partitions of devices.  The first sub-device
 * of device xxx will be xxx1, the second xxx2, ..., the 62nd xxxZ */
const char * const g_device_sub_names = "0123456789abcdefghijklmnopqrstuv"
                                        "wxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

dev_t *g_devices[MAX_DEVICES];

/*
    #include driver registration function declarations here
*/
#include "device/block/ata/ata.h"
#include "device/block/partition/partition.h"

block_driver_t *g_drivers[] =
{
    &g_ata_driver,
    &g_partition_driver
};


static dev_t *root_dev = NULL;


s32 dev_enumerate()
{
    /* Root device is implicit */
    root_dev = CHECKED_KCALLOC(1, sizeof(dev_t));

    /* Populating the device tree is a board-specific operation */
    return plat_dev_enumerate(root_dev);
}


dev_t *dev_get_root()
{
    return root_dev;
}


s32 dev_add_child(dev_t *parent, dev_t *child)
{
    /* TODO - mutex */

    /* Check that no device with a matching name exists */
    if(dev_find(child->name) != NULL)
        return EEXIST;

	child->parent = parent;

	if(parent->first_child == NULL)
		parent->first_child = child;
	else
	{
    	dev_t *p;

    	for(p = parent->first_child; p->next_sibling != NULL; p = p->next_sibling)
        	;

	    p->next_sibling = child;
    	child->prev_sibling = p;
	}

    return SUCCESS;
}


/*
    dev_find() - find a device by name
*/
dev_t *dev_find(const char * const name)
{
    return dev_find_subtree(name, root_dev);
}


/*
    dev_find_subtree() - find a device by name, starting at a particular point in the device tree
*/
dev_t *dev_find_subtree(const char * const name, dev_t *node)
{
    /* TODO - mutex */
    /* Walk the device tree looking for a device with the specified name */

    if(node == NULL)
        return NULL;
    else if(!strcmp(node->name, name))
        return node;
    else if(node->first_child)
        return dev_find_subtree(name, node->first_child);
    else if(node->next_sibling)
        return dev_find_subtree(name, node->next_sibling);
    else
        return NULL;
}


/*
    dev_add_suffix() - given a device name prefix, e.g. "ata", obtain the next-available complete
    device name, e.g. "ata1".  "name" must point to a writeable buffer of at least DEVICE_NAME_LEN
    chars.
*/
s32 dev_add_suffix(char * const name)
{
    u32 i;
    ku32 len = strlen(name);
    name[len + 1] = '\0';

    for(i = 0; i < strlen(g_device_sub_names); ++i)
    {
        name[len] = g_device_sub_names[i];
        if(dev_find(name) == NULL)
            return SUCCESS;
    }

    return EMFILE;
}


/*
	dev_create() - create a new device
*/
s32 dev_create(dev_type_t type, dev_subtype_t subtype, const char * const name, ku32 irql,
				void *base_addr, dev_t **dev)
{
	*dev = (dev_t *) CHECKED_KCALLOC(1, sizeof(dev_t));
	(*dev)->type		= type;
	(*dev)->subtype		= subtype;
	(*dev)->irql		= irql;
	(*dev)->base_addr	= base_addr;

	strcpy((*dev)->name, name);

    return dev_add_suffix((*dev)->name);
}


/*
    dev_register() - create and initialise a new device
*/
s32 dev_register(const dev_type_t type, const dev_subtype_t subtype, const char * const dev_name,
                 ku32 irql, void *base_addr, dev_t **dev, const char * const human_name,
                 dev_t *parent_dev, s32 (*init_fn)(dev_t *))
{
    s32 ret;

    ret = dev_create(type, subtype, dev_name, irql, base_addr, dev);
    if(ret != SUCCESS)
    {
        printf("%s: %s device creation failed: %s\n", dev_name, human_name, kstrerror(ret));
        return ret;
    }

    ret = init_fn(*dev);
    if(ret != SUCCESS)
    {
        kfree(*dev);
        printf("%s: %s device init failed: %s\n", dev_name, human_name, kstrerror(ret));
        return ret;
    }

    ret = dev_add_child(parent_dev, *dev);
    if(ret != SUCCESS)
    {
        /* FIXME: de-init device */
        kfree(*dev);
        printf("%s: failed to add %s to device tree: %s\n", dev_name, human_name, kstrerror(ret));
        return ret;
    }

    return SUCCESS;
}


u32 driver_init()
{
    u32 x;
	for(x = 0; x < (sizeof(g_drivers) / sizeof(g_drivers[0])); ++x)
	{
	    block_driver_t * const drv = g_drivers[x];

		if(drv->init() != SUCCESS)	/* init() will create devices */
		{
			printf("%s: driver initialisation failed\n", drv->name);

			/* Ensure that none of this driver's methods are callable */
		    drv->read = driver_method_not_implemented;
		    drv->write = driver_method_not_implemented;
		    drv->shut_down = driver_method_not_implemented;
		    drv->control = driver_method_not_implemented;
		}
	}

	return SUCCESS;
}


/*
    Generic function which can be used by drivers to indicate that a particular operation isn't
    supported.  Also, if a driver fails to initialise, all of its operation funcptrs will be
    replaced with a call to this fn.
*/
s32 driver_method_not_implemented()
{
    return ENOSYS;
}


void driver_shut_down()
{
	/* TODO: ensure all devices are stopped/flushed/etc */
	u32 x;

	const u32 num_drivers = sizeof(g_drivers) / sizeof(g_drivers[0]);
	for(x = 0; x < num_drivers; ++x)
	{
		if(g_drivers[x]->shut_down() != SUCCESS)
		{
			/* TODO: error */
			printf("Failed to shut down '%s' driver\n", g_drivers[x]->name);
		}
	}

	for(x = 0; x < (sizeof(g_devices) / sizeof(g_devices[0])); ++x)
    {
        if(g_devices[x])
            kfree(g_devices[x]);
    }
}


dev_t *get_device_by_name(const char * const name)
{
	int i;
	for(i = 0; i < (sizeof(g_devices) / sizeof(g_devices[0])); ++i)
	{
	    if(g_devices[i] != NULL)
        {
            if(!strcmp(g_devices[i]->name, name))
                return g_devices[i];
        }
	}

	return NULL;	/* No such device */
}


/* Create (i.e. register) a new device.  This function is called by each driver's init() function
 * during device enumeration. */
s32 create_device(const dev_type_t type, const dev_subtype_t subtype, void * const driver,
                  const char *name, void * const data)
{
    u32 u;
    dev_t * dev;

    /* Find a vacant device ID */
    for(u = 0; u < MAX_DEVICES; ++u)
    {
        if(g_devices[u] == NULL)
            break;
    }

    if(u == MAX_DEVICES)
        return ENFILE;      /* Too many devices */

    dev = (dev_t *) kmalloc(sizeof(dev_t));
    if(dev == NULL)
        return ENOMEM;

	dev->type = type;
	dev->subtype = subtype;
	dev->driver = driver;
	dev->data = data;

	/* TODO: check that the name is not a duplicate */
	strncpy(dev->name, name, sizeof(dev->name));
    g_devices[u] = dev;

	return SUCCESS;
}


s32 device_read(const dev_t * const dev, ku32 offset, u32 len, void *buf)
{
	return ((block_driver_t *) dev->driver)->read(dev->data, offset, len, buf);
}


s32 device_write(const dev_t * const dev, ku32 offset, u32 len, const void *buf)
{
	return ((block_driver_t *) dev->driver)->write(dev->data, offset, len, buf);
}


s32 device_control(const dev_t * const dev, ku32 function, void *in, void *out)
{
	return ((block_driver_t *) dev->driver)->control(dev->data, function, in, out);
}

