#include "memory/memorymap.h"

.text
.org 0

# supervisor stack pointer - place this at the top of kernel RAM
.int OS_STACK_TOP

# reset vector
.int _main

