/*
    string.c - implementation of various libc string-related functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include "string.h"


/*
    memcmp()
*/
s32 memcmp(const void *s1, const void *s2, u32 n)
{
    u8 *s1_ = (u8 *) s1,
       *s2_ = (u8 *) s2;

	if(!n)
		return 0;

	for(; n-- && (*s1_ == *s2_); ++s1_, ++s2_)
		if(*s1_ == 0)
			return 0;

	return *s1_ - *s2_;
}


/*
    memcpy()
*/
void *memcpy(void *dest, const void *src, u32 n)
{
	s8 *src_ = (s8 *) src,
		 *dest_ = (s8 *) dest;

    if(!n)
        return dest;

    if(((u32) src_ & 3) == ((u32) dest_ & 3))
    {
        /* src and dest can be aligned on a word boundary. */
        u32 nwords;

        while(n-- & ((u32) src_ & 3))
            *dest_++ = *src_++;

        for(nwords = n >> 2; nwords--; src_ += 4, dest_ += 4)
            *((u32 *) dest_) = *((u32 *) src_);

        for(n &= 3; n--;)
            *dest_++ = *src_++;
    }
    else
    {
        /* src and dest can't be aligned.  Do this the slow way */
        for(; n; --n)
            *dest_++ = *src_++;
    }

	return dest;
}


/*
    memset()
*/
void *memset(void *src, s32 c, s32 n)
{
    u8 *src_ = (u8 *) src;
    while(n--)
        *src_++ = c;

    return src;
}


/*
    strcat()
*/
s8 *strcat(s8 *dest, ks8 *src)
{
	s8 *dest_ = dest + strlen(dest);
	strcpy(dest_, src);

	return dest;
}


/*
    strchr()
*/
s8 *strchr(s8 *s, s32 c)
{
    for(; (*s != c) && *s; ++s)
        ;

    return s;
}


/*
    strcmp()
*/
s32 strcmp(ks8 *s1, ks8 *s2)
{
	for(; *s1 == *s2; ++s1, ++s2)
		if(*s1 == 0)
			return 0;
	return *(u8 *) s1 - *(u8 *) s2;
}


/*
    strcpy()
*/
s8 *strcpy(s8 *dest, ks8 *src)
{
	do
	{
		*dest++ = *src;
	} while(*src++);

	return dest;
}


/*
    strdup()
*/
s8 *strdup(ks8 *s)
{
	ku32 len = strlen(s);
	s8 *buf = malloc(len + 1);

	if(buf == NULL)
	{
		/* TODO: set error code */
		return 0;
	}

	strcpy(buf, s);

	return buf;
}


/*
    strerror()
*/
ks8 *strerror(ks32 errnum)
{
	switch(errnum)
	{
		case 0:					return "Success";
		case EPERM:				return "Operation not permitted";
		case ENOENT:			return "No such file or directory";
		case ESRCH:				return "No such process";
		case EINTR:				return "Interrupted system call";
		case EIO:				return "Input/output error";
		case ENXIO:				return "No such device or address";
		case E2BIG:				return "Argument list too long";
		case ENOEXEC:			return "Exec format error";
		case EBADF:				return "Bad file descriptor";
		case ECHILD:			return "No child processes";
		case EAGAIN:			return "Resource temporarily unavailable";
		case ENOMEM:			return "Cannot allocate memory";
		case EACCES:			return "Permission denied";
		case EFAULT:			return "Bad address";
		case ENOTBLK:			return "Block device required";
		case EBUSY:				return "Device or resource busy";
		case EEXIST:			return "File exists";
		case EXDEV:				return "Invalid cross-device link";
		case ENODEV:			return "No such device";
		case ENOTDIR:			return "Not a directory";
		case EISDIR:			return "Is a directory";
		case EINVAL:			return "Invalid argument";
		case ENFILE:			return "Too many open files in system";
		case EMFILE:			return "Too many open files";
		case ENOTTY:			return "Inappropriate ioctl for device";
		case ETXTBUSY:			return "Text file busy";
		case EFBIG:				return "File too large";
		case ENOSPC:			return "No space left on device";
		case ESPIPE:			return "Illegal seek";
		case EROFS:				return "Read-only file system";
		case EMLINK:			return "Too many links";
		case EPIPE:				return "Broken pipe";
		case EDOM:				return "Numerical argument out of domain";
		case ERANGE:			return "Numerical result out of range";
		case EDEADLK:			return "Resource deadlock avoided";
		case ENAMETOOLONG:		return "File name too long";
		case ENOLCK:			return "No locks available";
		case ENOSYS:			return "Function not implemented";
		case ENOTEMPTY:			return "Directory not empty";
		case ELOOP:				return "Too many levels of symbolic links";
		/*  41 Unknown error 41 */
		case ENOMSG:			return "No message of desired type";
		case EIDRM:				return "Identifier removed";
		case ECHRNG:			return "Channel number out of range";
		case EL2NSYNC:			return "Level 2 not synchronized";
		case EL3HLT:			return "Level 3 halted";
		case EL3RST:			return "Level 3 reset";
		case ELNRNG:			return "Link number out of range";
		case EUNATCH:			return "Protocol driver not attached";
		case ENOCSI:			return "No CSI structure available";
		case EL2HLT:			return "Level 2 halted";
		case EBADE:				return "Invalid exchange";
		case EBADR:				return "Invalid request descriptor";
		case EXFULL:			return "Exchange full";
		case ENOANO:			return "No anode";
		case EBADRQC:			return "Invalid request code";
		case EBADSLT:			return "Invalid slot";
		/* 58 Unknown error 58 */
		case EBFONT:			return "Bad font file format";
		case ENOSTR:			return "Device not a stream";
		case ENODATA:			return "No data available";
		case ETIME:				return "Timer expired";
		case ENOSR:				return "Out of streams resources";
		case ENONET:			return "Machine is not on the network";
		case ENOPKG:			return "Package not installed";
		case EREMOTE:			return "Object is remote";
		case ENOLINK:			return "Link has been severed";
		case EADV:				return "Advertise error";
		case ESRMNT:			return "Srmount error";
		case ECOMM:				return "Communication error on send";
		case EPROTO:			return "Protocol error";
		case EMULTIHOP:			return "Multihop attempted";
		case EDOTDOT:			return "RFS specific error";
		case EBADMSG:			return "Bad message";
		case EOVERFLOW:			return "Value too large for defined data type";
		case ENOTUNIQ:			return "Name not unique on network";
		case EBADFD:			return "File descriptor in bad state";
		case EREMCHG:			return "Remote address changed";
		case ELIBACC:			return "Can not access a needed shared library";
		case ELIBBAD:			return "Accessing a corrupted shared library";
		case ELIBSCN:			return ".lib section in a.out corrupted";
		case ELIBMAX:			return "Attempting to link in too many shared libraries";
		case ELIBEXEC:			return "Cannot exec a shared library directly";
		case EILSEQ:			return "Invalid or incomplete multibyte or wide character";
		case ERESTART:			return "Interrupted system call should be restarted";
		case ESTRPIPE:			return "Streams pipe error";
		case EUSERS:			return "Too many users";
		case ENOTSOCK:			return "Socket operation on non-socket";
		case EDESTADDRREQ:		return "Destination address required";
		case EMSGSIZE:			return "Message too long";
		case EPROTOTYPE:		return "Protocol wrong type for socket";
		case ENOPROTOOPT:		return "Protocol not available";
		case EPROTONOSUPPORT:	return "Protocol not supported";
		case ESOCKTNOSUPPORT:	return "Socket type not supported";
		case EOPNOSUPPORT:		return "Operation not supported";
		case EPFNOSUPPORT:		return "Protocol family not supported";
		case EAFNOSUPPORT:		return "Address family not supported by protocol";
		case EADDRINUSE:		return "Address already in use";
		case EADDRNOTAVAIL:		return "Cannot assign requested address";
		case ENETDOWN:			return "Network is down";
		case ENETUNREACH:		return "Network is unreachable";
		case ENETRESET:			return "Network dropped connection on reset";
		case ECONNABORTED:		return "Software caused connection abort";
		case ECONNRESET:		return "Connection reset by peer";
		case ENOBUFS:			return "No buffer space available";
		case EISCONN:			return "Transport endpoint is already connected";
		case ENOTCONN:			return "Transport endpoint is not connected";
		case ESHUTDOWN:			return "Cannot send after transport endpoint shutdown";
		case ETOOMANYREFS:		return "Too many references: cannot splice";
		case ETIMEDOUT:			return "Connection timed out";
		case ECONNREFUSED:		return "Connection refused";
		case EHOSTDOWN:			return "Host is down";
		case EHOSTUNREACH:		return "No route to host";
		case EALREADY:			return "Operation already in progress";
		case EINPROGRESS:		return "Operation now in progress";
		case ESTALE:			return "Stale NFS file handle";
		case EUCLEAN:			return "Structure needs cleaning";
		case ENOTNAM:			return "Not a XENIX named type file";
		case ENAVAIL:			return "No XENIX semaphores available";
		case EISNAM:			return "Is a named type file";
		case EREMOTEIO:			return "Remote I/O error";
		case EDQUOT:			return "Disk quota exceeded";
		case ENOMEDIUM:			return "No medium found";
		case EMEDIUMTYPE:		return "Wrong medium type";
		case ECANCELED:			return "Operation canceled";
		case ENOKEY:			return "Required key not available";
		case EKEYEXPIRED:		return "Key has expired";
		case EKEYREVOKED:		return "Key has been revoked";
		case EKEYREJECTED:		return "Key was rejected by service";
		case EOWNERDEAD:		return "Owner died";
		case ENOTRECOVERABLE:	return "State not recoverable";
		case ERFKILL:			return "Operation not possible due to RF-kill";

		default:				return "Unknown error";
	}
}


/*
    strlen()
*/
u32 strlen(const s8 *s)
{
	const s8 *s_ = s;
	for(s_ = s; *s; ++s) ;
	return s - s_;
}


/*
    strncmp()
*/
s32 strncmp(ks8 *s1, ks8 *s2, u32 n)
{
	if(!n)
		return 0;

	for(; n && (*s1 == *s2); ++s1, ++s2, --n)
		if(*s1 == 0)
			return 0;

	return n ? *s1 - *s2 : 0;
}


/*
    strncpy()
*/
s8 *strncpy(s8 *dest, ks8 *src, u32 n)
{
	u32 n_;

	for(n_ = 0; (n_ < n) && (src[n_] != '\0'); n_++)
		dest[n_] = src[n_];

	for(; n_ < n; n_++)
		dest[n_] = '\0';

	return dest;
}


/*
    strrchr()
*/
s8 *strrchr(s8 *s, s32 c)
{
    s8 *p;

    for(p = NULL; *s; ++s)
        if(*s == c)
            p = s;

    return p;
}


/*
    strstr()
*/
s8 *strstr(ks8 *haystack, ks8 *needle)
{
	ks8 *n;
	for(n = needle; *haystack; ++haystack)
	{
		if(*haystack != *n)
			n = needle;

		if(*haystack == *n)
			if(!*++n)
				return (s8 *) haystack - strlen(needle) + 1;
	}

	return NULL;
}
