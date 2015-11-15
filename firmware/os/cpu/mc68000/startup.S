#include <kernel/memory/memorymap.h>

.text
.org 0

# supervisor stack pointer - place this at the top of kernel RAM
# FIXME: OS_STACK_TOP comes from kernel/memory/memorymap.h, and is platform-specific
.int OS_STACK_TOP

# reset vector
.int _main

