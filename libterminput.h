/* See LICENSE file for copyright and license details. */
#ifndef LIBTERMINPUT_H
#define LIBTERMINPUT_H

#include <stddef.h>


enum libterminput_mod {
	LIBTERMINPUT_SHIFT = 0x01,
	LIBTERMINPUT_META  = 0x02,
	LIBTERMINPUT_CTRL  = 0x04
};

enum libterminput_key {
	LIBTERMINPUT_SYMBOL,
	LIBTERMINPUT_UP,
	LIBTERMINPUT_DOWN,
	LIBTERMINPUT_RIGHT,
	LIBTERMINPUT_LEFT,
	LIBTERMINPUT_BEGIN, /* keypad 5 without numlock */
	LIBTERMINPUT_TAB,   /* backtab if with shift */
	LIBTERMINPUT_F1,
	LIBTERMINPUT_F2,
	LIBTERMINPUT_F3,
	LIBTERMINPUT_F4,
	LIBTERMINPUT_F5,
	LIBTERMINPUT_F6,
	LIBTERMINPUT_F7,
	LIBTERMINPUT_F8,
	LIBTERMINPUT_F9,
	LIBTERMINPUT_F10,
	LIBTERMINPUT_F11,
	LIBTERMINPUT_F12,
	LIBTERMINPUT_HOME,  /* = find */
	LIBTERMINPUT_INS,
	LIBTERMINPUT_DEL,   /* = remove */
	LIBTERMINPUT_END,   /* = select */
	LIBTERMINPUT_PRIOR, /* page up   */
	LIBTERMINPUT_NEXT,  /* page down */
	LIBTERMINPUT_ERASE, /* backspace */
	LIBTERMINPUT_ENTER, /* return    */
	LIBTERMINPUT_ESC,
	LIBTERMINPUT_KEYPAD_0,
	LIBTERMINPUT_KEYPAD_1,
	LIBTERMINPUT_KEYPAD_2,
	LIBTERMINPUT_KEYPAD_3,
	LIBTERMINPUT_KEYPAD_4,
	LIBTERMINPUT_KEYPAD_5,
	LIBTERMINPUT_KEYPAD_6,
	LIBTERMINPUT_KEYPAD_7,
	LIBTERMINPUT_KEYPAD_8,
	LIBTERMINPUT_KEYPAD_9,
	LIBTERMINPUT_KEYPAD_PLUS,
	LIBTERMINPUT_KEYPAD_MINUS,
	LIBTERMINPUT_KEYPAD_TIMES,
	LIBTERMINPUT_KEYPAD_DIVISION,
	LIBTERMINPUT_KEYPAD_DECIMAL,
	LIBTERMINPUT_KEYPAD_COMMA,
	LIBTERMINPUT_KEYPAD_POINT,
	LIBTERMINPUT_KEYPAD_ENTER,
};

enum libterminput_type {
	LIBTERMINPUT_NONE,
	LIBTERMINPUT_KEYPRESS,
	LIBTERMINPUT_BRACKETED_PASTE_START,
	LIBTERMINPUT_BRACKETED_PASTE_END,
	LIBTERMINPUT_TEXT
};

struct libterminput_keypress {
	enum libterminput_type type;
	enum libterminput_key key;
	unsigned long long int times; /* if .times > 1, next will be the same, but will .times -= 1 */
	enum libterminput_mod mods;
	char symbol[7];               /* use if .key == LIBTERMINPUT_SYMBOL */
};

struct libterminput_text {
	enum libterminput_type type;
	size_t nbytes;
	char bytes[512];
};

union libterminput_input {
	enum libterminput_type type;
	struct libterminput_keypress keypress; /* use if .type == LIBTERMINPUT_KEYPRESS */
	struct libterminput_text text;         /* use if .type == LIBTERMINPUT_TEXT */
};


/**
 * This struct should be considered opaque
 */
struct libterminput_state {
	int inited; /* whether the input in initialised, not this struct */
	enum libterminput_mod mods;
	size_t stored_head;
	size_t stored_tail;
	char bracketed_paste;
	char meta;
	char n;
	char npartial;
	char partial[7];
	char key[44];
	char stored[512];
};


/**
 * Get input from the terminal
 * 
 * @param   fd     The file descriptor to the terminal
 * @param   input  Output parameter for input
 * @param   ctx    State for the terminal, parts of the state may be stored in `input`
 * @return         1 normally, 0 on end of input, -1 on error
 */
int libterminput_read(int fd, union libterminput_input *input, struct libterminput_state *ctx);


#endif
