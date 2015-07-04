ram_start = 0x00000400
ram_end   = 0x000ffffc


.text
.org 0

# supervisor stack pointer
.int 0x100000

# reset vector
.int _main

