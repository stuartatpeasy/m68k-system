/*
    stdlib.c - implementation of various libc functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include <klibc/include/stdlib.h>
#include <klibc/include/errno.h>


static s32 g_rand_next = 1;


/*
    calloc()
*/
void *calloc(ku32 nmemb, ku32 size)
{
    return kcalloc(nmemb, size);
}


/*
    free()
*/
void free(void *ptr)
{
    kfree(ptr);
}


/*
    malloc() - allocate *kernel* memory
*/
void *malloc(u32 size)
{
    return kmalloc(size);
}


/*
    path_canonicalise() - canonicalise <path> by removing duplicate dir-separator ('/') characters,
    resolving references to './' and '../', and trimming trailing '/' characters.  The result is
    stored in <path>, and a pointer to this argument is returned.  Note that <path> should contain
    an absolute path, to avoid unusual behaviour.

    Note: this is not a standard libc function.
*/
char *path_canonicalise(char *path)
{
    /* Iterate over components in path */
    ks8 *read_start, *read_end;
    s8 *write;

    read_start = read_end = write = path;

    while(*read_start)
    {
        /* Walk over directory-separator ('/') chars */
        if(*read_end == DIR_SEPARATOR)
        {
            *write++ = '/';
            while(*++read_end == DIR_SEPARATOR)
                ;

            read_start = read_end;
        }

        /* Extract path component */
        while(*read_end && (*read_end != DIR_SEPARATOR))
            ++read_end;

        if(read_start != read_end)
        {
            ku32 len = read_end - read_start;

            if((len == 1) && (read_start[0] == '.'))
            {
                /* This component is a "." - ignore it. */
                if(write > path)
                    --write;

                read_start = read_end;
            }
            else if((len == 2) && (read_start[0] == '.') && (read_start[1] == '.'))
            {
                /* Rewind the write pointer past the previous path component, if any */
                write = ((write - 2) >= path) ? write - 2 : path;

                while((write > path) && (*--write != DIR_SEPARATOR))
                    ;

                read_start = read_end;
            }
            else
            {
                /* Copy the component to the output string */
                while(read_start < read_end)
                    *write++ = *read_start++;
            }
        }
    }

    /* Trim trailing '/', if present. */
    if((write > path) && (*(write - 1) == DIR_SEPARATOR))
        --write;

    *write = '\0';

    return path;
}


/*
    rand()
*/
s32 rand()
{
    g_rand_next = g_rand_next * RAND_LCG_MULTIPLIER + RAND_LCG_INCREMENT;

    /* No scaling needed here as long as RAND_MAX = S32_MAX */
    return g_rand_next;
}


/*
    rand32() [non-standard]
*/
s32 rand32()
{
    g_rand_next = g_rand_next * RAND_LCG_MULTIPLIER + RAND_LCG_INCREMENT;
    return g_rand_next;
}


/*
    realloc()
*/
void *realloc(void *ptr, u32 size)
{
    return krealloc(ptr, size);
}


/*
    srand()
*/
void srand(ks32 seed)
{
    g_rand_next = seed;
}


/*
    strtoul()
*/
u32 strtoul(ks8 *nptr, s8 **endptr, s32 base)
{
    u32 n = 0;
    u64 n_;
    u8 digit = 0;
    s8 digit_found = 0, overflow = 0, neg = 0;
    ks8 * const nptr_ = nptr;

    /* skip over whitespace */
    while((*nptr == ' ') || (*nptr == '\t') || (*nptr == '\r') || (*nptr == '\n'))
        ++nptr;

    /* any -ve number equates to zero */
    if(*nptr == '-')
    {
        neg = 1;
        ++nptr;
    }

    /* read optional '+' character */
    else if(*nptr == '+')
        ++nptr;

    /* check that base is valid, i.e. it lies between 2 and 36. */
    if(base && ((base < 2) || (base > 36)))
    {
        errno = EINVAL;
        return 0;
    }

    /*
        if base is zero:
            - look for a "0x" prefix: if present, assume base 16
            - look for a "0" prefix: if present, assume base 8
            - else assume base 10.
    */
    if(!base)
    {
        if(*nptr == '0')
        {
            if((*(nptr + 1) == 'x') || (*(nptr + 1) == 'X'))
                base = 16;
            else
                base = 8;
        }
        else
            base = 10;
    }

    /* if base == 16 and there is a "0x" or "0X" prefix, step over the prefix. */
    if((base == 16) && (*nptr == '0') && ((*(nptr + 1) == 'x') || (*(nptr + 1) == 'X')))
        nptr += 2;

    /* process numeric characters */
    for(; *nptr; ++nptr)
    {
        if((*nptr >= '0') && (*nptr <= '9'))
            digit = *nptr - '0';
        else if((*nptr >= 'a') && (*nptr <= 'z'))
            digit = (*nptr - 'a') + 10;
        else if((*nptr >= 'A') && (*nptr <= 'Z'))
            digit = (*nptr - 'A') + 10;
        else
            digit = 255;        /* flag this digit as invalid */

        if(digit >= base)
        {
            /* invalid digit */
            if(endptr)
            {
                if(digit_found)
                    *endptr = (s8 *) nptr;
                else
                    *endptr = (s8 *) nptr_;
            }
            return n;
        }
        else
            digit_found = 1;

        n_ = ((u64) n) * base;

        if(n_ > 0xffffffffULL)
            overflow = 1;

        n = (u32) n_ + digit;;
    }

    if(endptr)
        *endptr = (s8 *) nptr;

    if(overflow)
    {
        errno = ERANGE;
        return U32_MAX;
    }

    return neg ? -n : n;
}
