#include "display.h"

// {rom_address, code_point}
static const size_t offset[][2] = {
	{0x7FD10, 0},
	{0xBF640, 110},
	{0xE2920, 94 * 4},
	{0xE2BD0, 94 * 5},
	{0xE2C90, 502},
	{0xE2D50, 94 * 6},
	{0xE2E60, 94 * 7},
	{0xDEEA0, 612},
	{0xBFE50, 694},
	{0x3ABF0, 94 * 15},
    {0xF6F80, 94 * 15 + 0x5400 / BYTES_PER_GLYPH}
};

uint8_t font_not_found() {
	if(*MAGIC == 0xC0DEBA5E) {
		return 0;
	}
    gotoxy(2, 0);
    puts("please update firmware frist !");
	return 1;
}

void *get_bitmap_font(uint32_t *addr) {
    uint8_t ch_page = sat_peek_sat_byte(*addr); 
    uint8_t ch_id = sat_peek_sat_byte((*addr) + 2);

    int32_t page = ch_page - 0xA0;    //区码
    int32_t id = ch_id - 0xA0;  //位码

	if (page < 0) {               // ISO/IEC 2022 G0区
	    page = 3;                 // 用 GB 2312-1980 第3区中
	    id = ch_page - 0x20;    // 相应的全角字符代替半角字符
	    *addr += 2;              // 半角字符，指针移动一字节
	} else {                      // ISO/IEC 2022 GR区
	    *addr += 4;              // 全角字符，指针移动两字节
	}

	size_t code_point = ((page - 1) * 94 + (id - 1));
	for (size_t i = (sizeof(offset) / sizeof(*offset)) - 1; i >= 0; i--) {
		if (code_point >= offset[i][1]) {
			code_point -= offset[i][1];
			code_point *= BYTES_PER_GLYPH;
			code_point += offset[i][0];
			return (void *)code_point;
		}
	}
}

uint32_t g_x = 0, g_y = 0;

uint32_t display_font(uint32_t addr, uint32_t x, uint32_t y) {
    uint8_t pos = 7;
    uint8_t *ptr = (uint8_t *)get_bitmap_font(&addr);
    
    for (size_t row = 0; row < ROWS; row++, y++) {
        for (size_t col = 0; col < COLS_STORAGE; col++, x++) {
            __display_buf[y * BYTES_PER_ROW + (x >> 3)] |= ((*ptr >> pos) & 1) << (x & 7);
            if (pos) {
                pos--;
            } else {
                pos = 7;
                ptr++;
            }
        }
        x -= COLS_STORAGE; 
    }

    g_x = x;
    g_y = y;

	return addr;
}


uint32_t display_page(uint32_t addr, uint32_t end_addr) {

	_clear_screen();
    uint8_t ch = sat_peek_sat_byte(addr);

	if (font_not_found()) {
		return addr;
	} else if (ch == 0x0A) {
		//跳过页面开始的换行符
		addr += 2;
	}
    // x=3, y=1
	uint32_t x = LEFT_MARGIN, y = TOP_MARGIN;
    
    while(addr < end_addr) {
    
        ch = sat_peek_sat_byte(addr);
        //windows下的0x0D 0x0A
        //传输到hp39gs只剩下0x0A
        if (ch == 0x0A) {
            //遇到换行符切换至下一行
			addr += 2;
			x = SCREEN_WIDTH;
			y += ROWS;
		}else {
            addr = display_font(addr, x, y);
            x = g_x;
            y = g_y;
        } 
        
        if (x + COLS_REAL <= SCREEN_WIDTH - COLS_REAL) {
			//next char
			x += COLS_REAL;
			y -= ROWS;
		} else if (y + LINE_SPACING + ROWS <= SCREEN_HEIGHT) {
			//next line
			x = LEFT_MARGIN;
			y += LINE_SPACING;
		} else {
			//next page
			break;
		}
    }
    set_indicator(INDICATOR_RSHIFT, (addr < end_addr));
    return addr;
}