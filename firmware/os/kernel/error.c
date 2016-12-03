/*
    Kernel error-reporting functions

    Part of ayumos


    (c) Stuart Wallace, January 2016.

    For now this module provides an error-reporting abstraction.  I expect the interface to grow and
    become more useful in future.
*/

#include <kernel/error.h>
#include <kernel/include/cpu.h>
#include <klibc/stdio.h>


/*
    kernel_warning() - display a warning message on the current console
*/
void kernel_warning(const char * const msg)
{
    put("[WARNING] ");
    puts(msg);
}


/*
    kernel_fatal() - report a fatal error (on the current console) and halt.
*/
void kernel_fatal(const char * const msg)
{
    put("[FATAL] ");
    puts(msg);

    cpu_halt();
}
