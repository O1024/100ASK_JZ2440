/**
 * @file main.c
 * @brief Bootloader Entry Wrapper
 */

extern void boot_main(void);

int main(void) {
    boot_main();
    return 0;
}
