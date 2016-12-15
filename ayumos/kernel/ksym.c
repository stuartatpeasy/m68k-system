/*
	Kernel symbol table lookup functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <kernel/include/ksym.h>
#include <kernel/include/limits.h>
#include <klibc/include/stdio.h>
#include <klibc/include/string.h>

#define for_each_sym(sym) \
    for(sym = (symentry_t *) &_ssym; sym->len; sym = (symentry_t *) (sym->len + ((u8 *) sym)))


/*
    ksym_find_by_name() - look up a symbol by name.
*/
s32 ksym_find_by_name(const char * const name, symentry_t **ent)
{
#ifdef DEBUG_KSYM
    symentry_t *sym;

    for_each_sym(sym)
        if(!strcmp(&sym->name, name))
        {
            *ent = sym;
            return SUCCESS;
        }
#else
    UNUSED(name);
    UNUSED(ent);
#endif

    return ENOENT;
}


/*
    ksym_find_nearest_prev() - find the symbol whose address is closest to, but not greater than,
    addr.
*/
s32 ksym_find_nearest_prev(void *addr, symentry_t **ent)
{
#ifdef DEBUG_KSYM
    symentry_t *sym, *symfound = NULL;
    u32 dist = U32_MAX;

    for_each_sym(sym)
        if((addr >= sym->addr) && (dist >= (u32) (addr - sym->addr)))
        {
            dist = addr - sym->addr;
            symfound = sym;
        }

    if(symfound != NULL)
    {
        *ent = symfound;
        return SUCCESS;
    }
#else
    UNUSED(addr);
    UNUSED(ent);
#endif

    return ENOENT;
}


/*
    ksym_format_nearest_prev() - find the symbol whose address is closest to, but not greater than,
    addr; write the symbol name (and, if non-zero, its offset from addr) into the supplied buffer.
*/
s32 ksym_format_nearest_prev(void *addr, char *buf, u32 buf_len)
{
#ifdef DEBUG_KSYM
    symentry_t *ent;

    if(ksym_find_nearest_prev(addr, &ent) == SUCCESS)
    {
        if(ent->addr == addr)
            snprintf(buf, buf_len, "<%s>", &ent->name);
        else
            snprintf(buf, buf_len, "<%s+0x%x>", &ent->name, (u32) addr - (u32) ent->addr);

        return SUCCESS;
    }
#else
    UNUSED(addr);
#endif

    strncpy(buf, "<\?\?\?>", buf_len);  /* \-escape '?' chars to avoid trigraph warning */
    return ENOENT;
}
