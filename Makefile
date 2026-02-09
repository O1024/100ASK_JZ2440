CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

CFLAGS  = -march=armv4t -Wall -Iinclude -nostdlib -fno-builtin -g -O2
LDFLAGS = -T scripts/jz2440.lds

OBJS    = src/cpu/start.o src/main.o

all: jz2440.bin

jz2440.bin: jz2440.elf
	$(OBJCOPY) -O binary -S $< $@

LIBGCC  = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

jz2440.elf: $(OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBGCC) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) *.elf *.bin

.PHONY: all clean