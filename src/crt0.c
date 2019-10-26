#include <hpsys.h>
//#include "verify.h"
#include "user_main.h"

extern unsigned int *__exit_stk_state;
extern int _exit_save(unsigned int *ptr);
extern void __exit_cleanup();

uint8_t *__display_buf;

void _start(void) {
	volatile unsigned *LCDSADDR1 = (unsigned *)0x7300014;
	__display_buf = (uint8_t *)((*LCDSADDR1 & 0xffff) * 2 + 0x7f00000);
	
	unsigned state_buffer[4], lcd_buffer[17];
	__exit_stk_state = state_buffer;

	sys_intOff();
	sys_lcdsave(lcd_buffer);

	if (_exit_save((unsigned *)state_buffer)) {
        //if(check_serial()) {
            user_init(NULL);
        //}
		__exit_cleanup();
	}

	sys_lcdrestore(lcd_buffer);
	sys_intOn();
}
