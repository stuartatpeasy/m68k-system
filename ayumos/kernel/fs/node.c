/*
    File system node and node-cache management

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, February 2017.
*/

#include <kernel/include/fs/node.h>
#include <kernel/include/memory/slab.h>
#include <kernel/include/error.h>
#include <klibc/include/string.h>


/*
    fs_node_alloc() - allocate memory at <*node> to hold a fs_node_t struct.  This function does not
    allocate space for the node's <name> field.
*/
s32 fs_node_alloc(fs_node_t **node)
{
    fs_node_t *node_;
    s32 ret;

    ret = slab_alloc(sizeof(fs_node_t), (void **) &node_);
    if(ret != SUCCESS)
        return ret;

    node_->name = NULL;
    *node = node_;

    return SUCCESS;
}


/*
    fs_node_set_name() - duplicate the string <name> and store it inside <node>, first freeing any
    existing name buffer in <node>.
*/
s32 fs_node_set_name(fs_node_t *node, const char * const name)
{
    char *name_ = strdup(name);

    if(name_ == NULL)
        return ENOMEM;

    if(node->name != NULL)
        kfree(node->name);

    node->name = name_;
    return SUCCESS;
}


/*
    fs_node_free() - release the storage associated with <node>.
*/
void fs_node_free(fs_node_t *node)
{
    kfree(node->name);                  /* FIXME - use a slab to hold the name */
    slab_free(node);
}


/*
    fs_node_perm_str() build in str a ten-character "permission string", e.g. "drwxr-x---" from the
    supplied node.  str must point to a buffer of at least 10 characters.

    TODO: this probably shouldn't be here; put it somewhere else.
*/
s8 *fs_node_perm_str(const fs_node_t * const node, s8 *str)
{
    const file_perm_t perm = node->permissions;
    const fsnode_type_t type = node->type;

    str[0] = (type == FSNODE_TYPE_DIR) ? 'd' : '-';
    str[1] = (perm & FS_PERM_UR) ? 'r' : '-';
    str[2] = (perm & FS_PERM_UW) ? 'w' : '-';
    str[3] = (perm & FS_PERM_UX) ?
                ((perm & FS_PERM_UT) ? 's' : 'x') :
                ((perm & FS_PERM_UT) ? 'S' : '-');
    str[4] = (perm & FS_PERM_GR) ? 'r' : '-';
    str[5] = (perm & FS_PERM_GW) ? 'w' : '-';
    str[6] = (perm & FS_PERM_GW) ?
                ((perm & FS_PERM_GT) ? 's' : 'x') :
                ((perm & FS_PERM_GT) ? 'S' : '-');
    str[7] = (perm & FS_PERM_OR) ? 'r' : '-';
    str[8] = (perm & FS_PERM_OW) ? 'w' : '-';
    str[9] = (perm & FS_PERM_OX) ?
                ((perm & FS_PERM_OT) ? 's' : 'x') :
                ((perm & FS_PERM_OT) ? 'S' : '-');

    return str;
}

