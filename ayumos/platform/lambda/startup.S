.text
.org 0

# supervisor stack pointer - place this at the top of kernel RAM
.int 0x00040000

# reset vector
.int _main

