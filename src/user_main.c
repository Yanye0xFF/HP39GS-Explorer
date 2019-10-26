#include "user_main.h"

void display_title(const char *str);
void display_item(unsigned int count, SAT_OBJ_DSCR * obj);

int dispatch_file(SAT_OBJ_DSCR * obj, SAT_DIR_ENTRY * parent_dir);
int note_viewer(SAT_OBJ_DSCR * obj, SAT_DIR_ENTRY * ref);
int image_viewer(SAT_OBJ_DSCR * obj, SAT_DIR_ENTRY * ref);

uint8_t *unpack_image_file(uint32_t sat_obj_addr, uint16_t *width, uint16_t *height);
void display_image(uint8_t *buffer, uint32_t height, uint32_t width, int32_t loffset, int32_t hoffset);

void putstr(const char *str);

uint32_t sat_file_size(uint32_t sat_obj_addr);
uint8_t sat_file_header(uint32_t sat_obj_addr);

uint32_t FILE_TYPE;
volatile uint8_t disp_state = 1;

int user_init(SAT_DIR_ENTRY * init) {
	display_title(" Filelist ");
	putchar('\n');
    
	if (!init) {
		SAT_DIR_NODE *dir = _sat_find_path("/'notesdir");
		init = dir->object;
	} else {
		set_indicator(INDICATOR_LSHIFT, TRUE);
	}
    //TODO FIX
	unsigned count = 0;
	SAT_DIR_ENTRY *next_page = NULL;
	for (SAT_DIR_ENTRY * entry = init; entry; entry = entry->next) {
		SAT_OBJ_DSCR *obj = entry->sat_obj;
		if (obj->name[0] == ';') {
			continue;
		}
		if (count == 8) {
			next_page = entry;
			set_indicator(INDICATOR_RSHIFT, TRUE);
			break;
		}
		count++;
		display_item(count, obj);
	}
    int32_t key;
	static NODE *head;
	while(1) {
		key = get_key();
		if (key == KEY_EVENT_HOME) {
            while (head) {
				pop(&head);
			}
			return 0;			
		} else if ((key == KEY_EVENT_RIGHT || key == KEY_EVENT_DOWN) && next_page) {
			push(&head, next_page);
			return user_init(next_page);	
		} else if (key == KEY_EVENT_LEFT || key == KEY_EVENT_UP) {
			pop(&head);
			return user_init(pop(&head));	
		} else if (key >= 1 && key <= count) {
			for (SAT_DIR_ENTRY * entry = init; entry; entry = entry->next) {
				SAT_OBJ_DSCR *obj = entry->sat_obj;
				if (obj->name[0] == ';') {
					continue;
				}
				key--;
				if (!key) {
					return dispatch_file(obj, head ? head->data : NULL);
				}
			}
		}
	}
}

/*
计算sat文件大小
sat_obj_addr前5半字为文件类型，后5半字为文件大小，小端模式
文件大小 = (后5半字 - 0x05) >> 1
@param sat_obj_addr 土星文件对象地址
*/
uint32_t sat_file_size(uint32_t sat_obj_addr) {
	return ((uint32_t) sat_peek_sat_addr(sat_obj_addr + 5) - 0x05) >> 1;
}

/*
获取文件内容首字节
sat_obj_addr 10半字之后为文件内容
@param sat_obj_addr 土星文件对象地址 
*/
uint8_t sat_file_header(uint32_t sat_obj_addr) {
    return sat_peek(sat_obj_addr + 10, 2);
}

/*
显示字符串,不换行
*/
void putstr(const char *str) {
	while(*str) 
		putchar(*str++);
}

void display_title(const char *str) {
    /*
    unsigned len = strlen(str) + 2;
	extern int __scr_w;
	int margin_right = (__scr_w - len) / 2, margin_left = __scr_w - len - margin_right;
    */
    int8_t margin_left = 11, margin_right = 12;
    _clear_screen();
    while (margin_left--) {
		putchar('\x7f');
	}
    //putchar(' ');
    putstr(str);
	//putchar(' ');
	while (margin_right--) {
		putchar('\x7f');
	}
    //取消状态条显示
    set_indicator(INDICATOR_LSHIFT, FALSE);
	set_indicator(INDICATOR_ALPHA, FALSE);
}


void display_item(unsigned int count, SAT_OBJ_DSCR * obj) {
    putstr(" [");
	putchar('0' + count);
	putstr("]");
	char *name = obj->name + 1;
	while (*name) {
		putchar(*name++);
	}
	char buf[7] = {0};
	utoa(sat_file_size(obj->addr), buf, 10);
	for (unsigned i = obj->name - name - strlen(buf) + 24; i > 0; i--) {
		putchar(' ');
	}
	putstr(buf);
    puts("bytes");
}

/*
按照文件头分发文件打开方式
*/
int dispatch_file(SAT_OBJ_DSCR * obj, SAT_DIR_ENTRY * parent_dir) {
    //check file header (1 byte)
    uint8_t header = sat_file_header(obj->addr);
    FILE_TYPE = (header == 0xFD) ? IMAGE_FILE : TEXT_FILE;   
    if(header == 0xFD) {
        return image_viewer(obj, parent_dir);
    }else {
        return note_viewer(obj, parent_dir);
    }
}

int note_viewer(SAT_OBJ_DSCR * obj, SAT_DIR_ENTRY * ref) {
    //GB2312编码范围:A1A1-FEFE,其中汉字的编码范围为B0A1-F7FE,
    //首字节0xB0-0xF7(对应区号:16-87), 尾字节0xA1-0xFE(对应位号:01-94)
    int32_t key = 0;

	STACK *cur = NULL;
    uint32_t next_page = obj->addr + 10;	
    uint32_t end_addr = sat_file_size(obj->addr) * 2 + next_page;
    //init display state flag
	disp_state = 1;
    
    goto disp_txt_tag;
    
	while(1) {
		key = get_key();
		if (key == KEY_EVENT_HOME) {
			stack_clear(cur);
			return 0;			
		}else if (key == KEY_EVENT_VIEWS) {
            //go back to the list
			stack_clear(cur);
            rLCDCON1 |= 0x1;
			return user_init(ref);	
		}else if (key == KEY_EVENT_ENTER) {
            if(disp_state) {
                while((rLCDCON1 >> 18) > 2);
                rLCDCON1 &= 0xFFFFFFFE;
            }else {
                rLCDCON1 |= 0x1;
            }
            disp_state = !disp_state;
		}
        //隐藏显示状态下不处理翻页事件
        if(disp_state == 0) continue; 
        //handler key event
        if((next_page < end_addr) && (key == KEY_EVENT_RIGHT || key == KEY_EVENT_DOWN || key == 2)) {
            //page down
			disp_txt_tag:
            cur = stack_push(cur, next_page);
			next_page = display_page(next_page, end_addr);
			set_indicator(INDICATOR_LSHIFT, (cur->prev != NULL));
		}else if((cur->prev != NULL) && (key == KEY_EVENT_LEFT || key == KEY_EVENT_UP || key == 5)) {
            //page up, pop current page addr
            cur = stack_pop(cur, &next_page);
            //get previous page addr
            cur = stack_pop(cur, &next_page);
			goto disp_txt_tag;
		}
	}
}

const uint8_t side_icon[] = {0x02, 0x07, 0x07};

void display_image(uint8_t *buffer, uint32_t height, uint32_t width, int32_t loffset, int32_t hoffset) {
    uint32_t scr_index = 0;
    int32_t drawY = 0, drawX = 0;
    uint8_t *ptr;
    float k = 0.0;
    //计算实际绘制区域
    drawY = (height > 64) ? 64 : height;
    drawX = ((width >> 3) > 16) ? 16 : (width >> 3);
    //填充图像数据
    for(uint32_t y = 0; y < drawY; y++) {
        scr_index = (y << 4) + (y << 2);
        ptr = buffer + ((y + hoffset) * (width >> 3) + loffset) ;
        for(uint32_t x = 0; x < drawX; x++) {
            __display_buf[scr_index++] = *ptr++;            
        }
    }
    //图片高度不足/正好一屏则不绘制滚动条
    if(height < 65) return;
    //计算垂直滚动条位置
    k = (float)(60 - 3) / (height - 64);
    drawY = k * (hoffset - 0) + 3;
    //绘制滚动条
    for(uint32_t y = 0; y < 64; y++) {
        scr_index = (y << 4) + (y << 2) + 16;
        if(y < 3) {
            __display_buf[scr_index] = side_icon[y];
        }else if(y > 60) {
            __display_buf[scr_index] = side_icon[63 - y]; 
        }else if(y == drawY) {
            __display_buf[scr_index] = side_icon[1];
        }else {
            __display_buf[scr_index] = 0x00;
        }           
    }
}

/*
* 解压图片文件，使用完毕务必释放buffer
* @param sat_obj_addr 图片文件地址
* @return buffer 解压后的文件数据
* @return *width 图片宽度
* @return *height 图片高度
*/
uint8_t *unpack_image_file(uint32_t sat_obj_addr, uint16_t *width, uint16_t *height) {
    uint32_t total = 0, file_size = 0;
    //calc image size
    *width = sat_peek(sat_obj_addr + 12, 4);
    *height = sat_peek(sat_obj_addr + 16, 4);
    total = (*width) * (*height) >> 3;
    //文件大小(n字节) = 实体大小(n字节) - 文件头(1字节) - 宽度(2字节) - 高度(2字节)
    file_size = sat_file_size(sat_obj_addr) - 0x05;
    //malloc memory
	uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * total);
    if(buffer == NULL) {
       return NULL;
    }
    //unpack image file
    volatile uint32_t i = 0, pos = 0;
    volatile uint8_t data = 0x00;
    volatile uint16_t count = 0x0000;
    while(i < file_size) {
        //20半字偏移量，其中sat文件类型占5半字，sat文件大小占5半字，图片文件头占1字节，宽度2字节，高度2字节
        //在读取时已将半字转换为字节,移位操作时以字节为单位
        data = sat_peek_sat_byte(sat_obj_addr + (i << 1) + 20);
        if(data == 0x00 || data == 0xFF) {
            count = sat_peek_sat_byte(sat_obj_addr + 20 + ((i + 1) << 1));
            if(count == 0x00) {
                count = sat_peek(sat_obj_addr + 20 + ((i + 2) << 1), 4);
            }
            //移位操作时以字节为单位
            i = (count > 0xFF) ? (i + 4) : (i + 2);
            for(uint32_t n = 0; n < count; n++) {
                *(buffer + pos) = data;
                pos++;
            }
        }else {
            *(buffer + pos) = data;
            pos++;
            //移位操作时以字节为单位
            i++;
        }
    }
    return buffer;
}

int image_viewer(SAT_OBJ_DSCR * obj, SAT_DIR_ENTRY * parent_dir) {
    int32_t key;
    uint16_t img_width = 0, img_height = 0;
    //init position
    volatile int32_t top = 0, left = 0, buttom = 0, right = 0;

    _clear_screen();
    //解压图片
    uint8_t *buffer = unpack_image_file(obj->addr, &img_width, &img_height);
    if(buffer == NULL) {
        //gotoxy(x,y); x, y为字符数, hpgcc默认minifont字体，宽高为4×5
        gotoxy(8, 5);
        puts("file too large !");
        delay(6250000);
        return user_init(parent_dir);
    }
    //calc margin
    buttom = (img_height > 64) ? (img_height - 64) : 0;
    right = (img_width > 128) ? ((img_width - 128) >> 3) : 0;
    //init display state flag
    disp_state = 1;
    goto disp_img_tag;
    //main loop
    while(1) {
        key = get_key();
        if(key == KEY_EVENT_HOME) {
            free(buffer);
            return 0;
        }else if(key == KEY_EVENT_VIEWS) {
            free(buffer);
            rLCDCON1 |= 0x1;
            return user_init(parent_dir);
        }else if(key == KEY_EVENT_ENTER) {
            if(disp_state) {
                while((rLCDCON1 >> 18) > 2);
                rLCDCON1 &= 0xFFFFFFFE;
            }else {
                rLCDCON1 |= 0x1;
            }
            disp_state = !disp_state;
        }
        //隐藏显示状态下不处理画面移动事件
        if(disp_state == 0) continue; 
        //handler keyboard event
        if((key == KEY_EVENT_UP || key == 5) && top) {
			buttom += 4; top -= 4;
        }else if((key == KEY_EVENT_LEFT || key == 1) && left) {
            right++; left--;
        }else if((key == KEY_EVENT_DOWN || key == 2 )&& buttom) {
			buttom -= 4; top += 4;
        }else if((key == KEY_EVENT_RIGHT || key == 3) && right) {
            right--; left++;
        }
        //flush to screen
        disp_img_tag:
        display_image(buffer, img_height, img_width, left, top);
        set_indicator(INDICATOR_LSHIFT, left);
        set_indicator(INDICATOR_RSHIFT, right);
    }
}

/*
按键事件处理
@param col 按键列编号
@param row 按键行编号
*/
int event_handler(uint32_t col, uint32_t row) {
    //[APLET], [ON], [HOME]
	if ((col == 7 && row == 0) || (col == 4 && row == 6) || (col == 6 && row == 6)) {
		return KEY_EVENT_HOME;
	}
    set_indicator(INDICATOR_WAIT, TRUE);
    if (col == 7 && row == 1) {
        while (any_key_pressed);
        set_indicator(INDICATOR_WAIT, FALSE);
		return KEY_EVENT_VIEWS;				
	}  
    if(row == 6 && col == 0) {
        while (any_key_pressed);
        set_indicator(INDICATOR_WAIT, FALSE);
        return KEY_EVENT_ENTER;
    }
    //delay by file type: image@100ms, text@250ms
    delay(FILE_TYPE ? 625000 : 1563000);
    set_indicator(INDICATOR_WAIT, FALSE);
	//D-Pad [UP]: 200, [LEFT]: 201, [DOWN]: 202, [RIGHT]: 203
	if (col == 6 && row < 4) {
		return KEY_D_PAD + row;
	}
    if((col >= 1 && col <=3) && (row >= 3 && row <= 5)) {
        //number keys (0~9)
        return (6 - row) * 3 - col + 1;
    }
	//unhandled keys
	return KEY_EVENT_NULL;
}
