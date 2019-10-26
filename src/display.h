#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define ROWS            8
#define COLS_STORAGE    8
#define COLS_REAL       9
#define BYTES_PER_GLYPH 8

#define SCREEN_WIDTH    131
#define SCREEN_HEIGHT   64
#define BYTES_PER_ROW   20

#define LEFT_MARGIN     3
#define TOP_MARGIN      1
#define LINE_SPACING    1

#define FILE_HEAD_MARGIN 5

#include <stdint.h>
#include <hpconio.h>
#include <saturn.h>

#define MAGIC ((unsigned *)0x0003FFF0)

extern uint8_t *__display_buf;

#define indicator(n) __display_buf[BYTES_PER_ROW * (n) + (SCREEN_WIDTH >> 3)]
#define INDICATOR_MASK (1 << (SCREEN_WIDTH & 7))
#define get_indicator(n) (indicator(n) | INDICATOR_MASK)
#define set_indicator(n, value) { \
	if (value) indicator(n) |= INDICATOR_MASK; \
	else indicator(n) &= ~INDICATOR_MASK; \
};

#define INDICATOR_REMOTE    0
#define INDICATOR_LSHIFT    1
#define INDICATOR_RSHIFT    2
#define INDICATOR_ALPHA     3
#define INDICATOR_BATTERY   4
#define INDICATOR_WAIT      5

uint32_t display_page(uint32_t addr, uint32_t end_addr);

#endif
