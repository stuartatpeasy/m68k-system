/*
    Implementations of MC68000-specific functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <cpu/cpu.h>
#include <cpu/mc68000/mc68000.h>


/*
    cpu_reset() - reset the MC68000.
*/
void cpu_reset(void)
{
    asm volatile
    (
        "reset                          \n"
        "lea.l      0x00f00000, %%a0    \n"     /* FIXME - board-specific ROM location */
        "move.l     %%a0@, %%a7         \n"
        "addq.l     #4, %%a0\n          \n"
        "move.l     %%a0@, %%a0         \n"
        "jmp        %%a0@               \n"
        :
        :
    );

    /* Won't return */
    while(1) ;
}


/*
    cpu_halt() - stop processing.
*/
void cpu_halt(void)
{
    /* the arg to "stop" causes the CPU to stay in supervisor mode and sets the IRQ mask to 7 */
    asm volatile
    (
        "stop       #0x2700             \n"
    );

    /* Won't return */
    while(1) ;
}


/*
    cpu_swi() - raise a software interrupt.
*/
void cpu_swi()
{
    asm volatile
    (
        "trap #15                       \n"
        :
        :
    );
}


u32 cpu_tas(void *addr)
{
    register u32 ret = 0;
    register u32 addr_ = (u32) addr;

    asm volatile
    (
        "      tas      %[address]      \n"
        "      bvc      L_%=            \n"
        "      moveq    #1, %[output]   \n"
        "L_%=:                          \n"
        : [output] "=&r" (ret)
        : [address] "d" (addr_)
        : "cc"
    );

    return ret;
}
