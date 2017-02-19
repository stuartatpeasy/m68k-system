/*
    device.c: device and device driver management

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 9th Febrary 2012.
*/

#include <klibc/include/errno.h>
#include <klibc/include/stdio.h>
#include <klibc/include/string.h>
#include <klibc/include/strings.h>

#include <kernel/include/device/device.h>
#include <kernel/include/device/partition.h>
#include <kernel/include/platform.h>
#include <kernel/util/kutil.h>


/*
    Characters used to identify "sub-devices", e.g. partitions of devices.  The first sub-device of
    device xxx will be xxx1, the second xxx2, ..., the 62nd xxxZ.  Similarly, the first sub-device
    of device xxx2 will be xxx20.
*/
const char * const g_device_sub_names = "0123456789abcdefghijklmnopqrstuv"
                                        "wxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static dev_t *root_dev = NULL;


/*
    dev_init() - initialise device support
*/
s32 dev_init()
{
    root_dev = kcalloc(1, sizeof(dev_t));

    /* FIXME - init root_dev as an actual device, duh. */

    return root_dev ? SUCCESS : -ENOMEM;
}


/*
    dev_enumerate() - enumerate platform and kernel devices
*/
s32 dev_enumerate()
{
    s32 ret;

    /* Populating the device tree is a board-specific operation */
    ret = plat_dev_enumerate();
    if(ret != SUCCESS)
        printf("Platform device enumeration failed: %s\nContinuing boot...", kstrerror(-ret));

    return ret;
}


/*
    dev_get_root() - obtain a ptr to the root element in the device tree
*/
dev_t *dev_get_root()
{
    return root_dev;
}


/*
    dev_add_child() - add a node to the device tree
*/
s32 dev_add_child(dev_t *parent, dev_t *child)
{
    /* TODO - mutex */
    /* Check that no device with a matching name exists */
    if(dev_find(child->name) != NULL)
        return -EEXIST;

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

    return -EMFILE;
}


/*
    dev_create() - create and initialise a new device
*/
s32 dev_create(const dev_type_t type, const dev_subtype_t subtype, const char * const dev_name,
                 ku32 irql, void *base_addr, dev_t **dev, const char * const human_name,
                 dev_t *parent_dev, s32 (*init_fn)(dev_t *))
{
    s32 ret;
    dev_t *d = (dev_t *) CHECKED_KCALLOC(1, sizeof(dev_t));

    d->type         = type;
    d->subtype      = subtype;
    d->state        = DEV_STATE_UNKNOWN;
    d->irql         = irql;
    d->base_addr    = base_addr;
    d->human_name   = human_name;
    d->data         = NULL;

    d->control      = dev_control_unimplemented;
    d->read         = dev_read_unimplemented;
    d->write        = dev_write_unimplemented;
    d->getc         = dev_getc_unimplemented;
    d->putc         = dev_putc_unimplemented;
    d->shut_down    = dev_shut_down_unimplemented;

    strcpy(d->name, dev_name);

    ret = dev_add_suffix(d->name);
    if(ret != SUCCESS)
    {
        kfree(d);
        return ret;
    }

    ret = dev_add_child(parent_dev, d);
    if(ret != SUCCESS)
    {
        printf("%s: failed to add %s to device tree: %s\n", d->name, human_name,
                kstrerror(-ret));
        kfree(d);
        return ret;
    }

    if(init_fn != NULL)
    {
        ret = init_fn(d);
        if(ret != SUCCESS)
        {
            printf("%s: %s device init failed: %s\n", d->name, human_name, kstrerror(-ret));
            dev_destroy(d);
            return ret;
        }
    }

    d->state = DEV_STATE_READY;

    if(dev != NULL)
        *dev = d;

    return SUCCESS;
}


/*
    dev_destroy() - destroy a device allocated with dev_create() or via dev_register().
*/
s32 dev_destroy(dev_t *dev)
{
    /* TODO: lock device tree */
    /* Destroy the device's child devices */
    while(dev->first_child)
        dev_destroy(dev->first_child);

    ks32 ret = dev->shut_down(dev);
    if((ret != SUCCESS) && (ret != -ENOSYS))
        return ret;

    if(dev->parent && (dev->parent->first_child == dev))
        dev->parent->first_child = dev->next_sibling;

    if(dev->prev_sibling)
        dev->prev_sibling->next_sibling = dev->next_sibling;

    if(dev->next_sibling)
        dev->next_sibling->prev_sibling = dev->prev_sibling;

    kfree(dev);

    return SUCCESS;
}


/*
    dev_get_type_char() - get the character code representing a high-level device type.
*/
char dev_get_type_char(const dev_t * const dev)
{
    switch(dev->type)
    {
        case DEV_TYPE_NONE:         return '-';
        case DEV_TYPE_BLOCK:        return 'b';
        case DEV_TYPE_CHARACTER:    return 'c';
        case DEV_TYPE_NET:          return 'n';
        case DEV_TYPE_SERIAL:       return 's';
        case DEV_TYPE_RTC:          return 'r';
        case DEV_TYPE_MEM:          return 'm';
        case DEV_TYPE_NVRAM:        return 'M';
        case DEV_TYPE_TIMER:        return 't';
        case DEV_TYPE_MULTI:        return 'x';
    }

    return '?';
}


/*
    dev_read_unimplemented() - default handler for dev->read() calls
*/
s32 dev_read_unimplemented(dev_t * const dev, ku32 offset, u32 *len, void *buf)
{
    UNUSED(dev);
    UNUSED(offset);
    UNUSED(len);
    UNUSED(buf);

    return -ENOSYS;
}


/*
    dev_write_unimplemented() - default handler for dev->write() calls
*/
s32 dev_write_unimplemented(dev_t * const dev, ku32 offset, u32 *len, const void *buf)
{
    UNUSED(dev);
    UNUSED(offset);
    UNUSED(len);
    UNUSED(buf);

    return -ENOSYS;
}


/*
    dev_control_unimplemented() - default handler for dev->control() calls
*/
s32 dev_control_unimplemented(dev_t * const dev, devctl_fn_t function, const void *in, void *out)
{
    UNUSED(dev);
    UNUSED(function);
    UNUSED(in);
    UNUSED(out);

    return -ENOSYS;
}


/*
    dev_getc_unimplemented() - default handler for dev->getc() calls
*/
s16 dev_getc_unimplemented(dev_t * const dev)
{
    UNUSED(dev);

    return -ENOSYS;
}


/*
    dev_putc_unimplemented() - default handler for dev->putc() calls
*/
s32 dev_putc_unimplemented(dev_t * const dev, const char c)
{
    UNUSED(dev);
    UNUSED(c);

    return -ENOSYS;
}


/*
    dev_shut_down_unimplemented() - default handler for dev->shut_down() calls
*/
s32 dev_shut_down_unimplemented(dev_t * const dev)
{
    UNUSED(dev);

    return -ENOSYS;
}
