/*
    Pre-emption enabling / disabling

    Part of ayumos


    (c) Stuart Wallace, December 2016.


    This is a very unsophisticated way of managing pre-emption.  Ayumos only supports single-
    processor systems, so all we're really doing is enabling and disabling CPU interrupts.
*/

#include <kernel/include/preempt.h>


/*
    The pre-emption counter.  Maintaining this counter enables nesting of calls to
    preempt_disable() / preempt_enable().

    We assume that this counter will never overflow.  More importantly, it is assumed that calls to
    preempt_disable() / preempt_enable() will be correctly nested.
*/
vu32 preempt_count = 0;
