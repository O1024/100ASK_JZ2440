/**
 * @file hal_lcd.c
 * @brief LCD HAL Implementation
 *
 * Generic logic layer. Calls LLD primitives.
 */

#include "hal/hal_lcd.h"
#include "ll_lcd.h"

void hal_lcd_init(void) {
    ll_lcd_init();
}

void hal_lcd_enable(int enable) {
    ll_lcd_enable(enable);
}

void hal_lcd_clear(uint16_t color) {
    uint16_t *fb = (uint16_t *)LCD_FB_BASE;
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        fb[i] = color;
    }
}

void hal_lcd_draw_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT)
        return;
    uint16_t *fb = (uint16_t *)LCD_FB_BASE;
    fb[y * LCD_WIDTH + x] = color;
}

void hal_lcd_draw_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            hal_lcd_draw_pixel(x + i, y + j, color);
        }
    }
}
