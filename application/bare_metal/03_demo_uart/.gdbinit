# GDB init script for IRAM debugging (0x40000000)
# Usage: gdb-multiarch -x .gdbinit 03_demo_uart.elf
# Or:    gdb-multiarch -x .gdbinit

set remotetimeout 120

# Load symbol table (if ELF provided on command line, this is optional)
file 03_demo_uart.elf

# Connect to OpenOCD
target extended-remote :3333

# Halt CPU
monitor halt

# Load binary to IRAM at 0x40000000
restore 03_demo_uart.bin binary 0x40000000

# Set PC to entry point
set $pc = 0x40000000

# Set breakpoint at _start
break _start

# Print status
echo \nProgram loaded to IRAM (0x40000000). PC set to 0x40000000.\n
echo Type 'continue' or 'c' to start execution.\n
# Uncomment to auto-start:
# continue
