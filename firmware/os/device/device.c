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
#include <device/partition.h>
#include <platform/platform.h>


/* Characters used to identify "sub-devices", e.g. partitions of devices.  The first sub-device
 * of device xxx will be xxx1, the second xxx2, ..., the 62nd xxxZ */
const char * const g_device_sub_names = "0123456789abcdefghijklmnopqrstuv"
                                        "wxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static dev_t *root_dev = NULL;


s32 dev_enumerate()
{
    s32 ret;

    /* Root device is implicit */
    root_dev = CHECKED_KCALLOC(1, sizeof(dev_t));

    /* Populating the device tree is a board-specific operation */
    ret = plat_dev_enumerate();
    if(ret != SUCCESS)
        printf("Platform device enumeration failed: %s\nContinuing boot...", kstrerror(ret));

    return partition_init();
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

    if(parent == NULL)
        parent = root_dev;

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
    dev_get_next() - used to iterate over all nodes in the device tree
*/
dev_t *dev_get_next(dev_t *node)
{
    if(node == NULL)
        return root_dev;

    if(node->first_child)
        return node->first_child;

    if(node->next_sibling)
        return node->next_sibling;
    else
    {
        while(node->prev_sibling)
            node = node->prev_sibling;

        do
        {
            node = node->parent;
        } while(node && !node->next_sibling);

        return node ? node->next_sibling : NULL;
    }
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
    dev_t *ret;

    if(node == NULL)
        return NULL;

    if(!strcmp(node->name, name))
        return node;

    if(node->first_child)
    {
        ret = dev_find_subtree(name, node->first_child);
        if(ret != NULL)
            return ret;
    }

    for(; (node = node->next_sibling) != NULL;)
    {
        ret = dev_find_subtree(name, node);
        if(ret != NULL)
            return ret;
    }

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

    if(init_fn != NULL)
    {
        ret = init_fn(*dev);
        if(ret != SUCCESS)
        {
            kfree(*dev);
            printf("%s: %s device init failed: %s\n", (*dev)->name, human_name, kstrerror(ret));
            return ret;
        }
    }

    ret = dev_add_child(parent_dev, *dev);
    if(ret != SUCCESS)
    {
        /* FIXME: de-init device */
        kfree(*dev);
        printf("%s: failed to add %s to device tree: %s\n", (*dev)->name, human_name,
                kstrerror(ret));
        return ret;
    }

    return SUCCESS;
}


s32 device_read(dev_t * const dev, ku32 offset, u32 len, void *buf)
{
	return ((block_ops_t *) dev->driver)->read(dev, offset, len, buf);
}


s32 device_write(dev_t * const dev, ku32 offset, u32 len, const void *buf)
{
	return ((block_ops_t *) dev->driver)->write(dev, offset, len, buf);
}


s32 device_control(dev_t * const dev, ku32 function, void *in, void *out)
{
	return ((block_ops_t *) dev->driver)->control(dev, function, in, out);
}

