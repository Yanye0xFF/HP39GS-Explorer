#include "hp39kbd.h"

int32_t get_key() {
	// wait until a key is pressed
    while (*GPGDAT != 0x7FFE && !any_key_pressed);
	// deal with the modifier keys
	if (comma_pressed) {
		return event_handler(4, 3);
	} else if (on_pressed) {
		return event_handler(4, 6);
	}
	for (unsigned c = 0; c < 8; c++) {
		// set the current column pin to output, others inputs
		*GPGCON = (1 << 16 << c * 2) | 0xAAA9;
		delay(800); // ~116us
		// skip this column if no key in it is pressed
		if (!any_key_pressed) {
			continue;
		}
		for (unsigned r = 0; r < 8; r++) {
			// check whether a row is active
			if (!(*GPGDAT & (1 << (r + 1)))) {
				// restore all column pins to output
				*GPGCON = 0x5555AAA9;
				delay(800); //~100us
				// return at the first detected key press
				return event_handler(c, r);
			}
		}
	}
	return KEY_EVENT_NULL;
}
