/**
 * @file hal_lcd.h
 * @brief Professional LCD HAL Interface for JZ2440
 */

#ifndef __HAL_LCD_H__
#define __HAL_LCD_H__

#include <stdint.h>

/* --- Display Geometry (4.3 inch TFT) --- */
#define LCD_WIDTH           480
#define LCD_HEIGHT          272
#define LCD_BPP             16

/* --- Memory Buffer --- */
/* FrameBuffer Address (Top of 64MB SDRAM) */
#define LCD_FB_BASE         0x33800000 

/* --- Professional Color Type --- */
typedef uint16_t hal_lcd_color_t;

/* Standard RGB565 Colors */
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_WHITE         0xFFFF
#define COLOR_BLACK         0x0000
#define COLOR_YELLOW        0xFFE0

/* --- Core API --- */
void hal_lcd_init(void);
void hal_lcd_enable(int enable);
void hal_lcd_clear(hal_lcd_color_t color);
void hal_lcd_draw_pixel(int x, int y, hal_lcd_color_t color);
void hal_lcd_draw_rect(int x, int y, int w, int h, hal_lcd_color_t color);

#endif /* __HAL_LCD_H__ */
