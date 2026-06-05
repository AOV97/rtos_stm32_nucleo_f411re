#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>
#include <stdbool.h>
#include "hal_i2c.h"

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64
#define SSD1306_PAGES   (SSD1306_HEIGHT / 8)  /* 8 pages of 8 rows each */

typedef struct {
    HAL_I2C_Handle *i2c;
    uint8_t         addr;
    uint8_t         fb[SSD1306_PAGES][SSD1306_WIDTH];
} SSD1306_Handle;

/**
 * @brief Sends the SSD1306 initialization sequence and clears the display.
 *
 * Configures the display for 3.3V single-supply operation (charge pump on),
 * horizontal addressing mode, and the correct segment/COM orientation for a
 * standard 128×64 module. Zeroes the local framebuffer. Must be called once
 * before any draw or flush calls.
 *
 * @param h     Pointer to an uninitialized SSD1306_Handle to set up.
 * @param i2c   Pointer to an already-initialized HAL_I2C_Handle for the bus
 *              this display is connected to.
 * @param addr  7-bit I2C address of the display (typically 0x3C or 0x3D).
 */
void ssd1306_init(SSD1306_Handle *h, HAL_I2C_Handle *i2c, uint8_t addr);

/**
 * @brief Clears the local framebuffer by zeroing all pixels.
 *
 * Does not write to the display. Call ssd1306_flush() after drawing to push
 * the cleared frame to the hardware.
 *
 * @param h  Pointer to an initialized SSD1306_Handle.
 */
void ssd1306_clear(SSD1306_Handle *h);

/**
 * @brief Sends the entire 1 KB framebuffer to the display over I2C.
 *
 * Resets the display's column and page address pointers to (0, 0), then
 * transmits all 8 pages as sequential I2C transactions (129 bytes each:
 * 1 control byte + 128 pixel bytes). Pixels become visible after this call.
 *
 * @param h  Pointer to an initialized SSD1306_Handle.
 */
void ssd1306_flush(SSD1306_Handle *h);

/**
 * @brief Sets or clears a single pixel in the framebuffer.
 *
 * Out-of-bounds coordinates are silently ignored. Changes are not visible
 * until ssd1306_flush() is called.
 *
 * @param h   Pointer to an initialized SSD1306_Handle.
 * @param x   Column (0–127, left to right).
 * @param y   Row (0–63, top to bottom).
 * @param on  true to light the pixel, false to clear it.
 */
void ssd1306_draw_pixel(SSD1306_Handle *h, uint8_t x, uint8_t y, bool on);

/**
 * @brief Draws one ASCII character using the built-in 5×8 pixel font.
 *
 * Each character occupies a 6-column × 8-row cell (5 glyph columns + 1 blank
 * spacer). Characters outside the printable ASCII range (0x20–0x7E) are
 * rendered as '?'. Changes are not visible until ssd1306_flush() is called.
 *
 * @param h     Pointer to an initialized SSD1306_Handle.
 * @param x     Starting column for the character (0–127).
 * @param page  Display page to draw on (0 = top row, 7 = bottom row). Each
 *              page is 8 pixels tall, matching the font height exactly.
 * @param c     ASCII character to draw.
 */
void ssd1306_draw_char(SSD1306_Handle *h, uint8_t x, uint8_t page, char c);

/**
 * @brief Draws a null-terminated string starting at the given position.
 *
 * Calls ssd1306_draw_char() for each character, advancing x by 6 pixels per
 * character. Stops when the string ends or the right edge of the display is
 * reached. Changes are not visible until ssd1306_flush() is called.
 *
 * @param h     Pointer to an initialized SSD1306_Handle.
 * @param x     Starting column (0–127).
 * @param page  Display page (0–7).
 * @param s     Null-terminated ASCII string to draw.
 */
void ssd1306_draw_string(SSD1306_Handle *h, uint8_t x, uint8_t page, const char *s);

#endif /* SSD1306_H_ */
