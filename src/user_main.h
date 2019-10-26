#ifndef __USER_MAIN_H__
#define __USER_MAIN_H__

#include <stdint.h>
#include <satdir.h>
#include <saturn.h>
#include <hpconio.h>
#include <hpstring.h>
#include <syscall.h>

#include "hp39kbd.h"
#include "stack.h"
#include "display.h"

#define TEXT_FILE 0
#define IMAGE_FILE 1

#define rLCDCON1 (*(volatile unsigned *)0x07300000)

extern uint8_t *__display_buf;
extern unsigned int FILE_TYPE;

int user_init(SAT_DIR_ENTRY *init);
int event_handler(unsigned col, unsigned row);

#endif
