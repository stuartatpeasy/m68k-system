/*
    mkromfs.c: build a romfs image from the supplied path

    Part of ayumos


    (c) Stuart Wallace, February 2017.

    Syntax: mkromfs [-l <volume_label>] -r <root_dir> [-o <image_file>]
            mkromfs -h
*/

#define TARGET_LITTLEENDIAN
#define HOST_HARNESS
#define WITH_FS_ROMFS

#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <kernel/include/byteorder.h>
#include <kernel/include/types.h>
#include <kernel/include/fs/node.h>


/* Meaningless "magic number" which validates a romfs superblock */
#define ROMFS_SUPERBLOCK_MAGIC      (0x6c1f39e4)

/* Flags associated with a romfs_node_t */
#define RN_FILE         (0x0001)    /* Node represents a file       */
#define RN_DIR          (0x0002)    /* Node represents a directory  */

typedef struct romfs_superblock
{
    u32     magic;              /* = ROMFS_SUPERBLOCK_MAGIC                         */
    u32     len;                /* Length of the entire romfs, including superblock */
    u32     cdate;              /* Timestamp at which the fs image was created      */
    u16     nnodes;             /* Number of romfs_node_t structures in the fs      */
    char    label[16];          /* Zero-terminated volume label (name) string       */
} romfs_superblock_t;

/* romfs_node_t: metadata for a "node", i.e. a directory entry, within the file system */
typedef struct romfs_node
{
    u16         id;             /* Unique identifier of this node                               */
    u16         parent_id;      /* Identifier of this node's parent                             */
    u32         size;           /* Size of this node's data block; zero for directory nodes     */
    u32         offset;         /* Offset of the data block, from the start of the superblock   */
    u32         name_offset;    /* Offset of the node name, from the start of the superblock    */
    u32         mdate;          /* Node last-modification timestamp                             */
    u16         uid;            /* Owner user ID                                                */
    u16         gid;            /* Owner group ID                                               */
    file_perm_t permissions;    /* Node permissions                                             */
    u16         flags;          /* Flags (see constants defined above)                          */
} romfs_node_t;


/* Program exit codes */
#define E_SUCCESS       (0)     /* Success exit code            */
#define E_SYNTAX        (1)     /* Command-line syntax error    */
#define E_FILE          (2)     /* File create/open error       */
#define E_IO            (3)     /* Read/write failed            */
#define E_MALLOC        (4)     /* Memory allocation failed     */
#define E_STAT          (5)     /* stat() failed                */
#define E_PRINTF        (6)     /* snprintf() failed            */

/* Internal configuration constants */
#define INITIAL_DATA_BUF_LEN    1048576UL       /* Initial size of file data buffer             */
#define INITIAL_NAMES_BUF_LEN   16384UL         /* Initial size of names buffer                 */
#define INITIAL_NODES_BUF_LEN   256UL           /* Initial size of nodes buffer                 */
#define DATA_BUF_INCREMENT      16384UL         /* Minimum amount by which to grow data buffer  */
#define NAMES_BUF_INCREMENT     4096UL          /* Minimum amount by which to grow names buffer */
#define NODES_BUF_INCREMENT     256UL           /* Amount by which to extend nodes buffer       */

#define LABEL_MAX_LEN           15              /* Max length of a volume label string */


int add_node(romfs_node_t *node);
int add_dir(const char *path, int parent_id);
unsigned int add_name(const char *name);
unsigned int add_data(const char *pathname, unsigned int size);
void checked_fwrite(const void *data, size_t len, FILE *fp);
void *checked_malloc(size_t size, const char *name);
int usage(const char *imagename);

int next_id = 0;

unsigned int data_len = 0;
unsigned int data_pos = 0;
unsigned char *data;

unsigned int names_len = 0;
unsigned int names_pos = 0;
char *names;

unsigned int nodes_pos = 0;
unsigned int nodes_len = 0;
romfs_node_t *nodes;


/*
    main() - entry point
*/
int main(int argc, char **argv)
{
    FILE *fp_out;
    DIR *dir;
    romfs_superblock_t sblk;
    u32 names_offset, data_offset, i;
    char label[LABEL_MAX_LEN + 1] = {0}, root[PATH_MAX + 1] = {0};
    int opt;

    fp_out = stdout;

    /* Process and validate command-line arguments */
    while((opt = getopt(argc, argv, "l:r:o:h")) != -1)
    {
        switch(opt)
        {
            case 'l':       /* "-l <label>" - specify volume label */
                if(strlen(optarg) > LABEL_MAX_LEN)
                    error(E_SYNTAX, 0, "Label too long (%d chars max)", LABEL_MAX_LEN);
                strcpy(label, optarg);
                break;

            case 'r':       /* "-r <rootdir>" - specify dir containing files to be added to image */
                if(strlen(optarg) > PATH_MAX)
                    error(E_SYNTAX, 0, "Path name too long");

                dir = opendir(optarg);
                if(!dir)
                    error(E_FILE, errno, "Unable to open root directory '%s'", optarg);

                closedir(dir);
                strcpy(root, optarg);
                break;

            case 'o':       /* "-o <outfile>" - specify the output image file name */
                fp_out = fopen(optarg, "w");
                if(!fp_out)
                    error(E_FILE, errno, "Unable to open output file '%s'", optarg);
                break;

            case 'h':       /* "-h" - show help text */
            default:
                return usage(basename(argv[0]));
        }
    }

    if(root[0] == '\0')
        error(E_SYNTAX, 0, "No root directory specified (use '-r')");

    /* Allocate initial file data buffer */
    data = checked_malloc(INITIAL_DATA_BUF_LEN, "data");
    data_len = INITIAL_DATA_BUF_LEN;

    /* Allocate initial names buffer */
    names = checked_malloc(INITIAL_NAMES_BUF_LEN, "names");
    names_len = INITIAL_NAMES_BUF_LEN;

    /* Allocate initial nodes buffer */
    nodes = checked_malloc(INITIAL_NODES_BUF_LEN * sizeof(romfs_node_t), "nodes");
    nodes_len = INITIAL_NODES_BUF_LEN;

    add_dir(root, next_id);

    sblk.magic      = N2BE32(ROMFS_SUPERBLOCK_MAGIC);
    sblk.len        = N2BE32(sizeof(romfs_superblock_t) + (nodes_pos * sizeof(romfs_node_t))
                                + names_pos + data_pos);
    sblk.cdate      = N2BE32(time(NULL));
    sblk.nnodes     = N2BE16(nodes_pos);
    strcpy(sblk.label, label);

    /* Fix up data and names offsets in nodes */
    data_offset = sizeof(romfs_superblock_t) + (nodes_pos * sizeof(romfs_node_t));
    names_offset = data_offset + data_pos;

    for(i = 0; i < nodes_pos; ++i)
    {
        nodes[i].offset = N2BE32(BE2N32(nodes[i].offset) + data_offset);
        nodes[i].name_offset = N2BE32(BE2N32(nodes[i].name_offset) + names_offset);
    }

    /* Write fs image */
    checked_fwrite(&sblk, sizeof(sblk), fp_out);
    checked_fwrite(nodes, sizeof(romfs_node_t) * nodes_pos, fp_out);
    checked_fwrite(data, data_pos, fp_out);
    checked_fwrite(names, names_pos, fp_out);

    free(nodes);
    free(names);
    free(data);
    fclose(fp_out);

    return 0;
}


/*
    add_dir() - add dir <path> to the romfs image, recursively adding sub-directories too.  Use
    <parent_id> as the node ID of <path>'s parent node.
*/
int add_dir(const char *path, int parent_id)
{
    char pathname[PATH_MAX + 1];
    DIR *dir;
    struct dirent *ent;
    struct stat statbuf;

    errno = 0;

    dir = opendir(path);
    if(!dir)
        error(E_FILE, errno, "Unable to open directory '%s'", path);

    while((ent = readdir(dir)) != NULL)
    {
        int ret, node_id;
        romfs_node_t node;

        if(!strcmp(".", ent->d_name) || !strcmp("..", ent->d_name))
            continue;

        if(snprintf(pathname, PATH_MAX, "%s/%s", path, ent->d_name) < 0)
            error(E_PRINTF, 0, "Failed to generate path string");

        ret = stat(pathname, &statbuf);
        if(ret != 0)
            error(E_STAT, errno, "Failed to stat() '%s'", pathname);

        node_id = ++next_id;

        node.id             = N2BE16(node_id);
        node.parent_id      = N2BE16(parent_id);
        node.name_offset    = N2BE32(add_name(ent->d_name));
        node.mdate          = N2BE32(statbuf.st_mtime);
        node.uid            = N2BE16(statbuf.st_uid);
        node.gid            = N2BE16(statbuf.st_gid);
        node.permissions    = N2BE16(statbuf.st_mode & 0x777);

        if(S_ISDIR(statbuf.st_mode))
        {
            node.flags  = N2BE16(RN_DIR);
            node.offset = 0;
            node.size   = 0;
        }
        else if(S_ISREG(statbuf.st_mode))
        {
            node.flags  = N2BE16(RN_FILE);
            node.offset = N2BE32(add_data(pathname, statbuf.st_size));
            node.size   = N2BE32(statbuf.st_size);
        }
        else
            continue;       /* Ignore anything that isn't a dir or a file */

        add_node(&node);

        if(S_ISDIR(statbuf.st_mode))
            add_dir(pathname, node_id);
    }

    closedir(dir);

    return 0;
}


/*
    add_node() - add a romfs_node_t structure to the nodes array, resizing the array if necessary.
*/
int add_node(romfs_node_t *node)
{
    if(nodes_pos == nodes_len)
    {
        nodes = realloc(nodes, (nodes_pos + NODES_BUF_INCREMENT) * sizeof(romfs_node_t));
        if(nodes == NULL)
            error(E_MALLOC, errno, "Failed to realloc() %ld bytes",
                    (nodes_pos + NODES_BUF_INCREMENT) * sizeof(romfs_node_t));

        nodes_len += NODES_BUF_INCREMENT;
    }

    memcpy(&nodes[nodes_pos++], node, sizeof(romfs_node_t));

    return 0;
}


/*
    add_name() - add a node name to the "names" string table, resizing the table if necessary.
    Return the offset of the name from the start of the string table.
*/
unsigned int add_name(const char *name)
{
    unsigned int name_start, len;

    name_start = names_pos;
    len = strlen(name) + 1;

    if(names_pos + len >= names_len)
    {
        unsigned int extend_by = MAX(NAMES_BUF_INCREMENT, len);

        name = realloc(names, names_pos + extend_by);
        if(names == NULL)
            error(E_MALLOC, errno, "Failed to realloc() %d bytes", names_pos + extend_by);

        names_len += extend_by;
    }

    strncpy(names + names_pos, name, len);
    names_pos += len;

    return name_start;
}


/*
    add_data() - append the contents of the file at <pathname> to the data area pointed to by
    <data>, resizing the area if necessary, and rounding the length of the file contents up to the
    next multiple of 4.  Return the offset of the file contents from the start of the data area.
*/
unsigned int add_data(const char *pathname, unsigned int size)
{
    FILE *fp;
    unsigned int size_rounded, data_start;

    fp = fopen(pathname, "r");
    if(!fp)
        error(E_FILE, errno, "Unable to open input file '%s'", pathname);

    data_start = data_pos;

    /* Round size up to the next multiple of four bytes */
    size_rounded = (size + 3) & ~4;

    /* Increase data buffer size if needed */
    if(data_pos + size_rounded >= data_len)
    {
        unsigned int extend_by = MAX(DATA_BUF_INCREMENT, size_rounded);

        data = realloc(data, data_pos + extend_by);
        if(data == NULL)
        {
            fclose(fp);
            error(E_MALLOC, errno, "Failed to realloc() %d bytes", data_pos + size_rounded);
        }

        data_len += extend_by;
    }

    if(fread(data + data_pos, 1, size, fp) != size)
    {
        fclose(fp);
        error(E_IO, errno, "Failed to read data from '%s'", pathname);
    }

    for(; size < size_rounded; ++size)
        data[data_pos + size] = 0;

    data_pos += size_rounded;
    fclose(fp);

    return data_start;
}


/*
    checked_fwrite() - do an fwrite(); abort the program with an error message if it fails.
*/
void checked_fwrite(const void *data, size_t len, FILE *fp)
{
    if(fwrite(data, 1, len, fp) != len)
        error(E_IO, errno, "Write failed");
}


/*
    checked_malloc() - attempt to allocate <size> bytes; abort the program with an error message if
    the allocation fails.
*/
void *checked_malloc(size_t size, const char *name)
{
    void *p = malloc(size);
    if(p == NULL)
        error(E_MALLOC, 0, "Failed to allocate %d bytes for %s buffer", (int) size, name);

    return p;
}


/*
    usage() - display usage information.
*/
int usage(const char *imagename)
{
    fprintf(stderr,
        "Usage: %s [-l <volume_label>] -r <root_dir> [-o <image_file>]\n"
        "       %s -h\n\n"
        "  -l <volume_label>   Set volume label to <volume_label>.\n"
        "  -r <root_dir>       Specify the directory containing the files to be added.\n"
        "                      to the romfs image.\n"
        "  -o <image_file>     Specify the file in which to write the image.  If no\n"
        "                      output file is specified, the image will be written to\n"
        "                      stdout.\n"
        "  -h                  Display this usage information.\n\n"
        "This tool builds romfs images from files and directories on the local\n"
        "file system.  It is part of ayumos.\n\n", imagename, imagename);

    return 0;
}
