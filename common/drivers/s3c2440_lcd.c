/**
 * @file s3c2440_lcd.c
 * @brief S3C2440 LCD Driver Implementation (Struct-based)
 */

#include "hal/hal_lcd.h"
#include "s3c2440_soc.h"

#define H_PW        41
#define H_BPD       2
#define H_FPD       2
#define V_PW        10
#define V_BPD       2
#define V_FPD       2
#define CLKVAL_VAL  4

void hal_lcd_init(void) {
    /* 1. GPIO Configuration */
    GPIO_PORT(PORT_C)->CON = 0xAAAAAAAA;
    GPIO_PORT(PORT_D)->CON = 0xAAAAAAAA;
    
    // GPG4: LCD_PWREN
    GPIO_PORT(PORT_G)->CON |= (3 << 8);

    // GPB0: Backlight Control
    GPIO_PORT(PORT_B)->CON &= ~(3 << 0);
    GPIO_PORT(PORT_B)->CON |=  (1 << 0);
    GPIO_PORT(PORT_B)->DAT |=  (1 << 0);

    /* 2. LCD Controller Configuration */
    LCD->LCDCON1 = (CLKVAL_VAL << 8) | (3 << 5) | (12 << 1);
    LCD->LCDCON2 = ((V_BPD-1)<<24) | ((LCD_HEIGHT-1)<<14) | ((V_FPD-1)<<6) | ((V_PW-1)<<0);
    LCD->LCDCON3 = ((H_BPD-1)<<19) | ((LCD_WIDTH-1)<<8) | ((H_FPD-1)<<0);
    LCD->LCDCON4 = (H_PW-1);
    LCD->LCDCON5 = (1<<11) | (1<<10) | (1<<9) | (1<<8) | (1<<3) | (1<<0);

    /* 3. FrameBuffer Address */
    LCD->LCDSADDR1 = ((LCD_FB_BASE >> 22) << 21) | ((LCD_FB_BASE & 0x3FFFFF) >> 1);
    uint32_t fb_end = LCD_FB_BASE + LCD_WIDTH * LCD_HEIGHT * 2;
    LCD->LCDSADDR2 = ((fb_end & 0x3FFFFF) >> 1);
    LCD->LCDSADDR3 = (0 << 11) | (LCD_WIDTH);

    /* 4. Enable LCD Clock & Power */
    CLK_PWR->CLKCON |= (1 << 5);
    LCD->LCDCON1 |= 1;
}

void hal_lcd_enable(int enable) {
    if (enable) LCD->LCDCON1 |= 1;
    else LCD->LCDCON1 &= ~1;
}

void hal_lcd_clear(uint16_t color) {
    uint16_t *fb = (uint16_t *)LCD_FB_BASE;
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        fb[i] = color;
    }
}

void hal_lcd_draw_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;
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
