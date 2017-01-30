/*
    printf.c: definition of the *printf() family of functions, used in kernel debugging

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, June 2015.
*/

#include <klibc/include/stdio.h>
#include <klibc/include/string.h>


/*
    Size of the initial buffer used by vprintf() (and therefore printf()).  If this buffer is
    insufficient, a larger one will be allocated.
*/
#define PRINTF_INITIAL_BUFFER_SIZE  (128)

#define PF_HALF                 (0x01)
#define PF_LONG                 (0x02)
#define PF_LONG_LONG            (0x04)
#define PF_LEADING_ZERO         (0x08)
#define PF_HAVE_FIELD_WIDTH     (0x10)
#define PF_FIELD_DOT_SEEN       (0x20)
#define PF_NUM_OUTPUT_STARTED   (0x40)
#define PF_NEG_FIELD_WIDTH      (0x80)

static ku32 powers_of_ten[] =
{
             1UL,
            10UL,
           100UL,
          1000UL,
         10000UL,
        100000UL,
       1000000UL,
      10000000UL,
     100000000UL,
    1000000000UL
};

static ku64 powers_of_ten_ll[] =
{
                       1ULL,
                      10ULL,
                     100ULL,
                    1000ULL,
                   10000ULL,
                  100000ULL,
                 1000000ULL,
                10000000ULL,
               100000000ULL,
              1000000000ULL,
             10000000000ULL,
            100000000000ULL,
           1000000000000ULL,
          10000000000000ULL,
         100000000000000ULL,
        1000000000000000ULL,
       10000000000000000ULL,
      100000000000000000ULL,
     1000000000000000000ULL,
    10000000000000000000ULL
};

static ku16 npowers = sizeof(powers_of_ten) / sizeof(powers_of_ten[0]);


/*
    printf()
*/
s32 printf(const char *format, ...)
{
    va_list ap;
    s32 n;

    va_start(ap, format);
    n = vprintf(format, ap);
    va_end(ap);

    return n;
}


/*
    snprintf()
*/
s32 snprintf(char *str, u32 size, const char *format, ...)
{
    va_list ap;
    s32 n;

    va_start(ap, format);
    n = vsnprintf(str, size, format, ap);
    va_end(ap);

    return n;
}


/*
    sprintf()
*/
s32 sprintf(char *str, const char *format, ...)
{
    va_list ap;
    s32 n;

    va_start(ap, format);
    n = vsnprintf(str, S32_MAX, format, ap);
    va_end(ap);

    return n;
}


/*
    vprintf()

    Tries to print to a stack-based buffer of size PRINTF_INITIAL_BUFFER_SIZE; if this buffer
    isn't big enough, allocates a sufficiently-large heap-based buffer and prints to that instead.
*/
s32 vprintf(const char *format, va_list ap)
{
    s32 buf_size = PRINTF_INITIAL_BUFFER_SIZE;
    char *p;
    char buf[PRINTF_INITIAL_BUFFER_SIZE];
    s32 n;

    p = buf;
    while(1)
    {
        n = vsnprintf(p, buf_size, format, ap);

        if(n == -1)
            return n;

        if(n < buf_size)
        {
            put(p);
            if(p != buf)
                kfree(p);

            return n;
        }

        buf_size = n + 1;
        p = kmalloc(buf_size);
        if(!p)
            return -ENOMEM;
    }
}


/*
    vsprintf()
*/
s32 vsprintf(char *str, const char *format, va_list ap)
{
    return vsnprintf(str, S32_MAX, format, ap);     /* Assume an unlimited buffer */
}


/*
    vsnprintf()
*/


s32 vsnprintf(char *str, u32 size, const char *format, va_list ap)
{
    s32 flags, field_width, precision;
    u32 u, n = 0;
    const char *pc;
    const u32 size_ = size - 1;     /* # bytes available in buf, reserving space for trailing \0 */
    char ch;

    while((ch = *format))
    {
        if(ch == '%')
        {
            ch = *++format;
            if(!ch)
                break;
            else if(ch == '%')
            {
                if(n < size_)
                    str[n] = '%';
                ++n;
                break;
            }
            else if(ch == '-')
            {
                flags |= PF_NEG_FIELD_WIDTH;
                ch = *++format;
            }

            /* look for field-width specifier */
            flags = field_width = precision = 0;
            while(((ch >= '0') && (ch <= '9')) || (ch == '.'))
            {
                if(ch == '.')
                {
                    flags |= PF_FIELD_DOT_SEEN;
                }
                else
                {
                    if((ch == '0') && !(flags & PF_HAVE_FIELD_WIDTH))
                        flags |= PF_LEADING_ZERO;

                    flags |= PF_HAVE_FIELD_WIDTH;

                    if(flags & PF_FIELD_DOT_SEEN)
                        precision = (precision * 10) + (ch - '0');
                    else
                        field_width = (field_width * 10) + (ch - '0');
                }
                ch = *++format;
            }

            if(ch == 'l')
            {
                if(flags & PF_LONG)
                    flags |= PF_LONG_LONG;
                else
                    flags |= PF_LONG;
                ch = *++format;
            }
            else if(ch == 'h')
            {
                flags |= PF_HALF;
                ch = *++format;
            }

            if(ch == 's')
            {
                /* put string */
                s32 pad_len = 0;
                pc = va_arg(ap, const char *);

                if(field_width)
                {
                    pad_len = field_width - strlen(pc);

                    if(pad_len < 0)
                        pad_len = 0;

                    if(flags & PF_NEG_FIELD_WIDTH)
                    {
                        /* Left-pad to field_width with spaces */
                        for(; pad_len; --pad_len)
                            if(n < size_)
                                str[n++] = ' ';
                    }
                }

                if(precision)
                {
                    for(; *pc && precision--; ++pc)
                        if(n < size_)
                            str[n++] = *pc;
                }
                else
                {
                    for(; *pc; ++pc)
                        if(n < size_)
                            str[n++] = *pc;
                }

                if(field_width && !(flags & PF_NEG_FIELD_WIDTH))
                {
                    /* Right-pad to field_width with spaces */
                    for(; pad_len; --pad_len)
                        if(n < size_)
                            str[n++] = ' ';
                }
            }
            else if(ch == 'c')
            {
                if(n < size_)
                    str[n] = (char) va_arg(ap, int);
                ++n;
            }
            else if((ch == 'd') || (ch == 'i') || (ch == 'u'))
            {
                s32 req_digits, numeral;

                /* put decimal */
                if(!(flags & PF_LONG_LONG))
                {
                    /* Treat the arg as a long (32-bit) quantity */
                    u = va_arg(ap, u32);

                    if((flags & PF_HAVE_FIELD_WIDTH) && !field_width && !u)
                    {
                        /* Explicit zero-size field and zero-value arg; silently skip. */
                    }
                    else
                    {
                        /* if arg is -ve, convert it to +ve and output a '-' */
                        if((ch != 'u') && (u & 0x80000000))
                        {
                            u = ~u + 1;
                            if(n < size_)
                                str[n] = '-';
                            ++n;
                        }

                        /*
                            calculate the number of digits required to display the number. override
                            the width specifier if it does not allow sufficient digits.
                        */
                        for(req_digits = 0;
                            (req_digits < npowers) && (powers_of_ten[req_digits] <= u);
                            ++req_digits)
                            ;

                        if(!req_digits)
                            req_digits = 1;

                        if(req_digits > field_width)
                            field_width = req_digits;

                        /*
                            if leading zeroes or padding is enabled, and
                            field_width > req_digits, output leading zeroes
                        */
                        if(field_width > req_digits)
                        {
                            for(ch = (flags & PF_LEADING_ZERO) ? '0' : ' ';
                                field_width > req_digits; --field_width)
                            {
                                if(n < size_)
                                    str[n] = ch;
                                ++n;
                            }
                        }

                        for(--field_width; field_width >= 0; --field_width)
                        {
                            numeral = 0;
                            while(u >= powers_of_ten[field_width])
                            {
                                ++numeral;
                                u -= powers_of_ten[field_width];
                            }

                            if(n < size_)
                                str[n] = '0' + numeral;
                            ++n;
                        }
                    }
                }
                else
                {
                    /* Treat the arg as a long-long (64-bit) quantity */
                    u64 lu = va_arg(ap, u64);

                    if((flags & PF_HAVE_FIELD_WIDTH) && !field_width && !u)
                    {
                        /* Explicit zero-size field and zero-value arg; silently skip. */
                    }
                    else
                    {
                        /* if arg is -ve, convert it to +ve and output a '-' */
                        if((ch != 'u') && (lu & 0x8000000000000000ULL))
                        {
                            lu = ~lu + 1;
                            if(n < size_)
                                str[n] = '-';
                            ++n;
                        }

                        /*
                            calculate the number of digits required to display the number. override
                            the width specifier if it does not allow sufficient digits.
                        */
                        for(req_digits = 0;
                            (req_digits < npowers) && (powers_of_ten_ll[req_digits] <= lu);
                            ++req_digits)
                            ;

                        if(!req_digits)
                            req_digits = 1;

                        if(req_digits > field_width)
                            field_width = req_digits;

                        /*
                            if leading zeroes or padding is enabled, and
                            field_width > req_digits, output leading zeroes
                        */
                        if(field_width > req_digits)
                        {
                            for(ch = (flags & PF_LEADING_ZERO) ? '0' : ' ';
                                field_width > req_digits; --field_width)
                            {
                                if(n < size_)
                                    str[n] = ch;
                                ++n;
                            }
                        }

                        for(--field_width; field_width >= 0; --field_width)
                        {
                            numeral = 0;
                            while(lu >= powers_of_ten_ll[field_width])
                            {
                                ++numeral;
                                lu -= powers_of_ten[field_width];
                            }

                            if(n < size_)
                                str[n] = '0' + numeral;
                            ++n;
                        }
                    }
                }
            }
            else if((ch == 'x') || (ch == 'X') || (ch == 'p'))
            {
                const char *hex;
                u32 val;
                s32 nybble = 7;

                if((ch == 'p') && ((n + 1) < size_))
                {
                    if(!field_width)
                        field_width = 8;

                    str[n++] = '0';
                    str[n++] = 'x';
                    flags |= PF_LEADING_ZERO;
                }

                if(ch == 'X')
                    hex = "0123456789ABCDEF";
                else
                    hex = "0123456789abcdef";

                if(!field_width)
                    field_width = 1;
                else
                {
                    if(field_width > 8)
                        field_width = 8;

                    nybble = --field_width;
                    flags |= PF_NUM_OUTPUT_STARTED;
                }

                /* put hex val / ptr */
                if(!(flags & PF_LONG_LONG))
                {
                    /* Treat the arg as an unsigned long (32-bit) integer */
                    u = va_arg(ap, u32);
                    nybble = 28;    /* Offset of highest-order nybble, in bits) */
                    
                    for(; nybble >= 0; nybble -= 4)
                    {
                        val = (u >> nybble) & 0xf;
                        if(val || !nybble)
                        {
                            flags |= PF_NUM_OUTPUT_STARTED | PF_LEADING_ZERO;
                            if(n < size_)
                                str[n] = hex[val];
                            ++n;
                        }
                        else if(flags & PF_NUM_OUTPUT_STARTED)
                        {
                            if(n < size_)
                                str[n] = (flags & PF_LEADING_ZERO) ? '0' : ' ';
                            ++n;
                        }
                    }
                }
                else
                {
                    /* Treat the arg as an unsigned long-long (64-bit) integer */
                    u64 lu = va_arg(ap, u64);
                    nybble = 60;    /* Offset of highest-order nybble, in bits */
                    
                    for(; nybble >= 0; nybble -= 4)
                    {
                        val = (lu >> nybble) & 0xf;
                        if(val || !nybble)
                        {
                            flags |= PF_NUM_OUTPUT_STARTED | PF_LEADING_ZERO;
                            if(n < size_)
                                str[n] = hex[val];
                            ++n;
                        }
                        else if(flags & PF_NUM_OUTPUT_STARTED)
                        {
                            if(n < size_)
                                str[n] = (flags & PF_LEADING_ZERO) ? '0' : ' ';
                            ++n;
                        }
                    }
                }
            }
        } /* if(ch == '%') */
        else if(ch == '\\')
        {
            /* TODO: support e.g. \x<hex>, etc. */
            if((ch = *++format))
            {
                if(n < size_)
                    switch(ch = *++format)
                    {
                        case 'n':       str[n] = '\n';      break;
                        case 'r':       str[n] = '\r';      break;
                        case 't':       str[n] = '\t';      break;
                        case '0':       str[n] = '\0';      break;

                        default:        str[n] = ch;        break;
                    }

                ++n;
            }
        }
        else
        {
            if(n < size_)
                str[n] = ch;
            ++n;
        }

        ++format;
    }

    /* Write terminating \0 */
    str[(n <= size_) ? n : size_] = '\0';

    return n;
}
