#!/bin/bash
# check_binary_size.sh - Report binary size

if [ ! -f "spl.elf" ]; then
    exit 0
fi

# Determine media type (passed from Makefile)
MEDIA=${BOOT_MEDIA:-nand}

# Use arm-none-eabi-size to get segment sizes
SIZE_OUT=$(arm-none-eabi-size spl.elf | tail -n 1)
TEXT=$(echo $SIZE_OUT | awk '{print $1}')
DATA=$(echo $SIZE_OUT | awk '{print $2}')
BSS=$(echo $SIZE_OUT | awk '{print $3}')

TOTAL=$((TEXT + DATA + BSS))

echo "------------------------------------------------"
if [ "$MEDIA" = "nand" ]; then
    echo "SPL Resource Usage (NAND Mode - 4KB SRAM Limit)"
else
    echo "Bootloader Usage (NOR Mode - Direct Exec)"
fi
echo "------------------------------------------------"
echo "Code (Text):   $TEXT bytes"
echo "Data:          $DATA bytes"
echo "BSS:           $BSS bytes"
echo "Total Used:    $TOTAL bytes"
echo "------------------------------------------------"

if [ "$MEDIA" = "nand" ]; then
    LIMIT=4096
    REMAINING=$((LIMIT - TOTAL))
    if [ $REMAINING -lt 512 ]; then
        echo -e "\033[0;31mRemaining Stack: $REMAINING bytes (WARNING: Low safety margin!)\033[0m"
    else
        echo -e "\033[0;32mRemaining Stack: $REMAINING bytes (Safe)\033[0m"
    fi
else
    echo -e "\033[0;34mStack Status: Full 4KB SRAM available (Code runs in NOR)\033[0m"
fi
echo "------------------------------------------------"
