/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libterminput.h"


static const struct keypress {
	const char *part1;
	const char *part2;
	enum libterminput_key key;
	enum libterminput_mod mods;
	enum libterminput_flags flags;
} keypresses[] = {
	{"\033[[", "A", LIBTERMINPUT_F1, 0, 0},
	{"\033[[", "B", LIBTERMINPUT_F2, 0, 0},
	{"\033[[", "C", LIBTERMINPUT_F3, 0, 0},
	{"\033[[", "D", LIBTERMINPUT_F4, 0, 0},
	{"\033[[", "E", LIBTERMINPUT_F5, 0, 0},
	{"\033O", "A", LIBTERMINPUT_UP, 0, 0},
	{"\033O", "B", LIBTERMINPUT_DOWN, 0, 0},
	{"\033O", "C", LIBTERMINPUT_RIGHT, 0, 0},
	{"\033O", "D", LIBTERMINPUT_LEFT, 0, 0},
	{"\033O", "E", LIBTERMINPUT_BEGIN, 0, 0}, /* not attested */
	{"\033O", "F", LIBTERMINPUT_END, 0, 0},
	{"\033O", "G", LIBTERMINPUT_BEGIN, 0, 0}, /* not attested */
	{"\033O", "H", LIBTERMINPUT_HOME, 0, 0},
	{"\033O", "M", LIBTERMINPUT_KEYPAD_ENTER, 0, 0},
	{"\033O", "P", LIBTERMINPUT_F1, 0, 0},
	{"\033O", "Q", LIBTERMINPUT_F2, 0, 0},
	{"\033O", "R", LIBTERMINPUT_F3, 0, 0},
	{"\033O", "S", LIBTERMINPUT_F4, 0, 0},
	{"\033O", "p", LIBTERMINPUT_KEYPAD_0, 0, 0},
	{"\033O", "q", LIBTERMINPUT_KEYPAD_1, 0, 0},
	{"\033O", "r", LIBTERMINPUT_KEYPAD_2, 0, 0},
	{"\033O", "s", LIBTERMINPUT_KEYPAD_3, 0, 0},
	{"\033O", "t", LIBTERMINPUT_KEYPAD_4, 0, 0},
	{"\033O", "u", LIBTERMINPUT_KEYPAD_5, 0, 0},
	{"\033O", "v", LIBTERMINPUT_KEYPAD_6, 0, 0},
	{"\033O", "w", LIBTERMINPUT_KEYPAD_7, 0, 0},
	{"\033O", "x", LIBTERMINPUT_KEYPAD_8, 0, 0},
	{"\033O", "y", LIBTERMINPUT_KEYPAD_9, 0, 0},
	{"\033O", "k", LIBTERMINPUT_KEYPAD_PLUS, 0, 0},
	{"\033O", "m", LIBTERMINPUT_KEYPAD_MINUS, 0, 0},
	{"\033O", "j", LIBTERMINPUT_KEYPAD_TIMES, 0, 0},
	{"\033O", "o", LIBTERMINPUT_KEYPAD_DIVISION, 0, 0},
	{"\033O", "n", LIBTERMINPUT_KEYPAD_DECIMAL, 0, 0},
	{"\033O", "l", LIBTERMINPUT_KEYPAD_COMMA, 0, 0},
	{"\033O", "b", LIBTERMINPUT_KEYPAD_POINT, 0, 0},
	{"\033[", "A", LIBTERMINPUT_UP, 0, 0},
	{"\033[", "B", LIBTERMINPUT_DOWN, 0, 0},
	{"\033[", "C", LIBTERMINPUT_RIGHT, 0, 0},
	{"\033[", "D", LIBTERMINPUT_LEFT, 0, 0},
	{"\033[", "E", LIBTERMINPUT_BEGIN, 0, 0},
	{"\033[", "F", LIBTERMINPUT_END, 0, 0},
	{"\033[", "G", LIBTERMINPUT_BEGIN, 0, 0},
	{"\033[", "H", LIBTERMINPUT_HOME, 0, 0},
	{"\033[", "M", LIBTERMINPUT_MACRO, 0, LIBTERMINPUT_MACRO_ON_CSI_M},
	{"\033[", "P", LIBTERMINPUT_F1, 0, 0},
	{"\033[", "P", LIBTERMINPUT_PAUSE, 0, LIBTERMINPUT_PAUSE_ON_CSI_P},
	{"\033[", "Q", LIBTERMINPUT_F2, 0, 0},
	{"\033[", "R", LIBTERMINPUT_F3, 0, 0},
	{"\033[", "S", LIBTERMINPUT_F4, 0, 0},
	{"\033[", "U", LIBTERMINPUT_NEXT, 0, 0},
	{"\033[", "V", LIBTERMINPUT_PRIOR, 0, 0},
	{"\033[", "Z", LIBTERMINPUT_TAB, LIBTERMINPUT_SHIFT, 0},
	{"\033[", "Z", LIBTERMINPUT_BACKTAB, 0, LIBTERMINPUT_SEPARATE_BACKTAB},
	{"\033[", "a", LIBTERMINPUT_UP, LIBTERMINPUT_SHIFT, 0},
	{"\033[", "b", LIBTERMINPUT_DOWN, LIBTERMINPUT_SHIFT, 0},
	{"\033[", "c", LIBTERMINPUT_RIGHT, LIBTERMINPUT_SHIFT, 0},
	{"\033[", "d", LIBTERMINPUT_LEFT, LIBTERMINPUT_SHIFT, 0},
	{"\033[", "@", LIBTERMINPUT_INS, 0, LIBTERMINPUT_INS_ON_CSI_AT},
	{NULL, NULL, 0, 0, 0}
};

static const struct keynum {
	int number;
	enum libterminput_key key;
	enum libterminput_mod mods;
	enum libterminput_flags flags;
} keynums[] = {
	{1,  LIBTERMINPUT_HOME,  0, 0},
	{2,  LIBTERMINPUT_INS,   0, 0},
	{3,  LIBTERMINPUT_DEL,   0, 0},
	{4,  LIBTERMINPUT_END,   0, 0},
	{5,  LIBTERMINPUT_PRIOR, 0, 0},
	{6,  LIBTERMINPUT_NEXT,  0, 0},
	{7,  LIBTERMINPUT_HOME,  0, 0},
	{8,  LIBTERMINPUT_END,   0, 0},
	{9,  LIBTERMINPUT_ESC,   0, 0}, /* just made this one up */
	{11, LIBTERMINPUT_F1,    0, 0},
	{12, LIBTERMINPUT_F2,    0, 0},
	{13, LIBTERMINPUT_F3,    0, 0},
	{14, LIBTERMINPUT_F4,    0, 0},
	{15, LIBTERMINPUT_F5,    0, 0},
	{17, LIBTERMINPUT_F6,    0, 0},
	{18, LIBTERMINPUT_F7,    0, 0},
	{19, LIBTERMINPUT_F8,    0, 0},
	{20, LIBTERMINPUT_F9,    0, 0},
	{21, LIBTERMINPUT_F10,   0, 0},
	{23, LIBTERMINPUT_F11,   0, 0},
	{24, LIBTERMINPUT_F12,   0, 0},
	{25, LIBTERMINPUT_F1,    LIBTERMINPUT_SHIFT, 0},
	{26, LIBTERMINPUT_F2,    LIBTERMINPUT_SHIFT, 0},
	{28, LIBTERMINPUT_F3,    LIBTERMINPUT_SHIFT, 0},
	{29, LIBTERMINPUT_F4,    LIBTERMINPUT_SHIFT, 0},
	{31, LIBTERMINPUT_F5,    LIBTERMINPUT_SHIFT, 0},
	{32, LIBTERMINPUT_F6,    LIBTERMINPUT_SHIFT, 0},
	{33, LIBTERMINPUT_F7,    LIBTERMINPUT_SHIFT, 0},
	{34, LIBTERMINPUT_F8,    LIBTERMINPUT_SHIFT, 0},
	{0, 0, 0, 0}
};

static const struct mouse {
	const char *str;
	enum libterminput_event event;
	enum libterminput_button button;
	enum libterminput_mod mods;
	int x;
	int y;
} mice[] = {
	{"\033[32;1;2M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      0, 1, 2},
	{"\033[33;61;19M",     LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      0, 61, 19},
	{"\033[34;6115;1559M", LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      0, 6115, 1559},
	{"\033[35;0;0M",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      0, 1, 1},
	{"\033[36;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[37;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[38;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[39;5;6M",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[40;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_META, 5, 6},
	{"\033[41;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_META, 5, 6},
	{"\033[42;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_META, 5, 6},
	{"\033[43;5;6M",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_META, 5, 6},
	{"\033[44;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[45;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[46;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[47;5;6M",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[48;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[49;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[50;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[51;5;6M",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[52;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[53;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[54;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[55;5;6M",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[56;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[57;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[58;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[59;5;6M",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[64;5;6M",       LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON1,      0, 5, 6},
	{"\033[65;5;6M",       LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON2,      0, 5, 6},
	{"\033[66;5;6M",       LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON3,      0, 5, 6},
	{"\033[67;5;6M",       LIBTERMINPUT_MOTION,  LIBTERMINPUT_NO_BUTTON,    0, 5, 6},
	{"\033[68;5;6M",       LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[69;5;6M",       LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[70;5;6M",       LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[71;5;6M",       LIBTERMINPUT_MOTION,  LIBTERMINPUT_NO_BUTTON,    LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[96;1;2M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_SCROLL_UP,    0, 1, 2},
	{"\033[97;1;2M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_SCROLL_DOWN,  0, 1, 2},
	{"\033[98;1;2M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_SCROLL_LEFT,  0, 1, 2},
	{"\033[99;1;2M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_SCROLL_RIGHT, 0, 1, 2},
	{"\033[160;1;2M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1,     0, 1, 2},
	{"\033[161;1;2M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON2,     0, 1, 2},
	{"\033[162;1;2M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON3,     0, 1, 2},
	{"\033[163;1;2M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON4,     0, 1, 2},

	{"\033[<0;1;2M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      0, 1, 2},
	{"\033[<1;61;19M",     LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      0, 61, 19},
	{"\033[<2;6115;1559M", LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      0, 6115, 1559},
	{"\033[<4;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<5;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<6;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<8;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_META, 5, 6},
	{"\033[<9;5;6M",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_META, 5, 6},
	{"\033[<10;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_META, 5, 6},
	{"\033[<12;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<13;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<14;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<16;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[<17;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[<18;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[<20;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<21;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<22;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<24;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[<25;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[<26;5;6M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[<32;5;6M",      LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON1,      0, 5, 6},
	{"\033[<33;5;6M",      LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON2,      0, 5, 6},
	{"\033[<34;5;6M",      LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON3,      0, 5, 6},
	{"\033[<35;5;6M",      LIBTERMINPUT_MOTION,  LIBTERMINPUT_NO_BUTTON,    0, 5, 6},
	{"\033[<36;5;6M",      LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<37;5;6M",      LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<38;5;6M",      LIBTERMINPUT_MOTION,  LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<39;5;6M",      LIBTERMINPUT_MOTION,  LIBTERMINPUT_NO_BUTTON,    LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<64;1;2M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_SCROLL_UP,    0, 1, 2},
	{"\033[<65;1;2M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_SCROLL_DOWN,  0, 1, 2},
	{"\033[<66;1;2M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_SCROLL_LEFT,  0, 1, 2},
	{"\033[<67;1;2M",      LIBTERMINPUT_PRESS,   LIBTERMINPUT_SCROLL_RIGHT, 0, 1, 2},
	{"\033[<128;1;2M",     LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1,     0, 1, 2},
	{"\033[<129;1;2M",     LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON2,     0, 1, 2},
	{"\033[<130;1;2M",     LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON3,     0, 1, 2},
	{"\033[<131;1;2M",     LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON4,     0, 1, 2},

	{"\033[<0;1;2m",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      0, 1, 2},
	{"\033[<1;61;19m",     LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON2,      0, 61, 19},
	{"\033[<2;6115;1559m", LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON3,      0, 6115, 1559},
	{"\033[<4;5;6m",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<5;5;6m",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<6;5;6m",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<8;5;6m",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_META, 5, 6},
	{"\033[<9;5;6m",       LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_META, 5, 6},
	{"\033[<10;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_META, 5, 6},
	{"\033[<12;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<13;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<14;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<16;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[<17;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[<18;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_CTRL, 5, 6},
	{"\033[<20;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<21;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<22;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 5, 6},
	{"\033[<24;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[<25;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON2,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[<26;5;6m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON3,      LIBTERMINPUT_CTRL | LIBTERMINPUT_META, 5, 6},
	{"\033[<66;1;2m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_SCROLL_LEFT,  0, 1, 2},
	{"\033[<67;1;2m",      LIBTERMINPUT_RELEASE, LIBTERMINPUT_SCROLL_RIGHT, 0, 1, 2},
	{"\033[<128;1;2m",     LIBTERMINPUT_RELEASE, LIBTERMINPUT_XBUTTON1,     0, 1, 2},
	{"\033[<129;1;2m",     LIBTERMINPUT_RELEASE, LIBTERMINPUT_XBUTTON2,     0, 1, 2},
	{"\033[<130;1;2m",     LIBTERMINPUT_RELEASE, LIBTERMINPUT_XBUTTON3,     0, 1, 2},
	{"\033[<131;1;2m",     LIBTERMINPUT_RELEASE, LIBTERMINPUT_XBUTTON4,     0, 1, 2},

	{NULL, 0, 0, 0, 0, 0}
};


int
main(void)
{
#define TEST(EXPR)\
	do {\
		if (EXPR)\
			break;\
		fprintf(stderr, "Failure at line %i, with errno = %i (%s): %s\n", __LINE__, errno, strerror(errno), #EXPR);\
		exit(1);\
	} while (0)

#define TYPE_MEM(STR, LEN, T)\
	do {\
		alarm(5);\
		if (LEN)\
			TEST(write(fds[1], STR, (LEN)) == (LEN));\
		do {\
			TEST(libterminput_read(fds[0], &input, &ctx) == 1);\
		} while (input.type == LIBTERMINPUT_NONE && libterminput_is_ready(&input, &ctx));\
		TEST(input.type == (T));\
	} while (0)

#define TYPE(STR, T)\
	do {\
		alarm(5);\
		if ((STR) && *(const char *)(STR))\
			TEST(write(fds[1], STR, strlen(STR)) == (ssize_t)strlen(STR));\
		do {\
			TEST(libterminput_read(fds[0], &input, &ctx) == 1);\
		} while (input.type == LIBTERMINPUT_NONE && libterminput_is_ready(&input, &ctx));\
		TEST(input.type == (T));\
	} while (0)

#define KEYPRESS_(STR1, STR2, STR3, STR4, KEY, MODS, TIMES)\
	do {\
		int times__ = (TIMES);\
		alarm(5);\
		stpcpy(stpcpy(stpcpy(stpcpy(buffer, STR1), STR2), STR3), STR4);\
		if (*buffer)\
			TEST(write(fds[1], buffer, strlen(buffer)) == (ssize_t)strlen(buffer));\
		for (; times__; times__--) {\
			do {\
				TEST(libterminput_read(fds[0], &input, &ctx) == 1);\
			} while (input.type == LIBTERMINPUT_NONE && libterminput_is_ready(&input, &ctx));\
			TEST(input.type == LIBTERMINPUT_KEYPRESS);\
			TEST(input.keypress.key == (KEY));\
			TEST(input.keypress.mods == (MODS));\
			TEST(input.keypress.times == times__);\
		}\
	} while (0)

#define KEYPRESS(A, B, KEY, MODS)\
	do {\
		KEYPRESS_("",     A, "",    B, (KEY), (MODS), 1);\
		KEYPRESS_("",     A, "4",   B, (KEY), (MODS), 4);\
		KEYPRESS_("",     A, "1;1", B, (KEY), (MODS), 1);\
		KEYPRESS_("",     A, "1;2", B, (KEY), (MODS) | LIBTERMINPUT_SHIFT, 1);\
		KEYPRESS_("",     A, "1;3", B, (KEY), (MODS) | LIBTERMINPUT_META, 1);\
		KEYPRESS_("",     A, "1;4", B, (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META, 1);\
		KEYPRESS_("",     A, "1;5", B, (KEY), (MODS) | LIBTERMINPUT_CTRL, 1);\
		KEYPRESS_("",     A, "1;6", B, (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_CTRL, 1);\
		KEYPRESS_("",     A, "1;7", B, (KEY), (MODS) | LIBTERMINPUT_META | LIBTERMINPUT_CTRL, 1);\
		KEYPRESS_("",     A, "1;8", B, (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META | LIBTERMINPUT_CTRL, 1);\
		KEYPRESS_("",     A, "2;5", B, (KEY), (MODS) | LIBTERMINPUT_CTRL, 2);\
		KEYPRESS_("\033", A, "",    B, (KEY), (MODS) | LIBTERMINPUT_META, 1);\
		KEYPRESS_("\033", A, "4",   B, (KEY), (MODS) | LIBTERMINPUT_META, 4);\
		KEYPRESS_("\033", A, "1;1", B, (KEY), (MODS) | LIBTERMINPUT_META, 1);\
		KEYPRESS_("\033", A, "1;2", B, (KEY), (MODS) | LIBTERMINPUT_META | LIBTERMINPUT_SHIFT, 1);\
		KEYPRESS_("\033", A, "1;3", B, (KEY), (MODS) | LIBTERMINPUT_META, 1);\
		KEYPRESS_("\033", A, "1;4", B, (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META, 1);\
		KEYPRESS_("\033", A, "1;5", B, (KEY), (MODS) | LIBTERMINPUT_META | LIBTERMINPUT_CTRL, 1);\
		KEYPRESS_("\033", A, "1;6", B, (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META | LIBTERMINPUT_CTRL, 1);\
		KEYPRESS_("\033", A, "1;7", B, (KEY), (MODS) | LIBTERMINPUT_META | LIBTERMINPUT_CTRL, 1);\
		KEYPRESS_("\033", A, "1;8", B, (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META | LIBTERMINPUT_CTRL, 1);\
		KEYPRESS_("\033", A, "2;5", B, (KEY), (MODS) | LIBTERMINPUT_META | LIBTERMINPUT_CTRL, 2);\
	} while (0)

#define KEYNUM_(A, B, C, KEY, MODS)\
	do {\
		sprintf(numbuf, "%i", B);\
		KEYPRESS_(A, numbuf, C, "~", (KEY), (MODS), 1);\
		KEYPRESS_(A, numbuf, C, "^", (KEY), (MODS) | LIBTERMINPUT_CTRL, 1);\
		KEYPRESS_(A, numbuf, C, "$", (KEY), (MODS) | LIBTERMINPUT_SHIFT, 1);\
		KEYPRESS_(A, numbuf, C, "@", (KEY), (MODS) | LIBTERMINPUT_CTRL | LIBTERMINPUT_SHIFT, 1);\
	} while (0)

#define KEYNUM(NUM, KEY, MODS)\
	do {\
		KEYNUM_("\033[",     NUM, ";1", (KEY), (MODS));\
		KEYNUM_("\033[",     NUM, ";2", (KEY), (MODS) | LIBTERMINPUT_SHIFT);\
		KEYNUM_("\033[",     NUM, ";3", (KEY), (MODS) | LIBTERMINPUT_META);\
		KEYNUM_("\033[",     NUM, ";4", (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META);\
		KEYNUM_("\033[",     NUM, ";5", (KEY), (MODS) | LIBTERMINPUT_CTRL);\
		KEYNUM_("\033[",     NUM, ";6", (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_CTRL);\
		KEYNUM_("\033[",     NUM, ";7", (KEY), (MODS) | LIBTERMINPUT_META | LIBTERMINPUT_CTRL);\
		KEYNUM_("\033[",     NUM, ";8", (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META | LIBTERMINPUT_CTRL);\
		KEYNUM_("\033\033[", NUM, ";1", (KEY), (MODS) | LIBTERMINPUT_META);\
		KEYNUM_("\033\033[", NUM, ";2", (KEY), (MODS) | LIBTERMINPUT_META | LIBTERMINPUT_SHIFT);\
		KEYNUM_("\033\033[", NUM, ";3", (KEY), (MODS) | LIBTERMINPUT_META);\
		KEYNUM_("\033\033[", NUM, ";4", (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META);\
		KEYNUM_("\033\033[", NUM, ";5", (KEY), (MODS) | LIBTERMINPUT_META | LIBTERMINPUT_CTRL);\
		KEYNUM_("\033\033[", NUM, ";6", (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META | LIBTERMINPUT_CTRL);\
		KEYNUM_("\033\033[", NUM, ";7", (KEY), (MODS) | LIBTERMINPUT_META | LIBTERMINPUT_CTRL);\
		KEYNUM_("\033\033[", NUM, ";8", (KEY), (MODS) | LIBTERMINPUT_SHIFT | LIBTERMINPUT_META | LIBTERMINPUT_CTRL);\
	} while (0)

#define KEYPRESS_SPECIAL_CHAR(CHAR, KEY)\
	do {\
		buffer[0] = (CHAR);\
		buffer[1] = '\0';\
		TYPE(buffer, LIBTERMINPUT_KEYPRESS);\
		TEST(input.keypress.mods == 0);\
		TEST(input.keypress.times == 1);\
		TEST(input.keypress.key == (KEY));\
		buffer[0] = '\033';\
		buffer[1] = (CHAR);\
		buffer[2] = '\0';\
		TYPE(buffer, LIBTERMINPUT_KEYPRESS);\
		TEST(input.keypress.mods == LIBTERMINPUT_META);\
		TEST(input.keypress.times == 1);\
		TEST(input.keypress.key == (KEY));\
		buffer[0] = (char)((CHAR) | 0x80);\
		buffer[1] = '\0';\
		TYPE(buffer, LIBTERMINPUT_KEYPRESS);\
		TEST(input.keypress.mods == LIBTERMINPUT_META);\
		TEST(input.keypress.times == 1);\
		TEST(input.keypress.key == (KEY));\
	} while (0)

#define MOUSE(STR, EV, BTN, MODS, X, Y)\
	do {\
		TYPE(STR, LIBTERMINPUT_MOUSEEVENT);\
		TEST(input.mouseevent.button == (BTN));\
		TEST(input.mouseevent.mods == (MODS));\
		TEST(input.mouseevent.event == (EV));\
		TEST(input.mouseevent.x == (X));\
		TEST(input.mouseevent.y == (Y));\
	} while (0)

	char buffer[512], numbuf[3 * sizeof(int) + 2];
	struct libterminput_state ctx;
	union libterminput_input input;
	int fds[2];
	size_t i;

	memset(&ctx, 0, sizeof(ctx));
	TEST(!pipe(fds));

	for (i = 0; keypresses[i].part1; i++) {
		libterminput_set_flags(&ctx, keypresses[i].flags);
		KEYPRESS(keypresses[i].part1, keypresses[i].part2, keypresses[i].key, keypresses[i].mods);
		libterminput_clear_flags(&ctx, keypresses[i].flags);
	}

	for (i = 0; keynums[i].number; i++) {
		libterminput_set_flags(&ctx, keynums[i].flags);
		KEYNUM(keynums[i].number, keynums[i].key, keynums[i].mods);
		libterminput_clear_flags(&ctx, keynums[i].flags);
	}

	TYPE("\033[201~", LIBTERMINPUT_BRACKETED_PASTE_END);
	TYPE("x", LIBTERMINPUT_KEYPRESS);
	TYPE("\033[200~", LIBTERMINPUT_BRACKETED_PASTE_START);
	TYPE("x\033[201~", LIBTERMINPUT_TEXT);
	TEST(input.text.nbytes == strlen("x"));
	TEST(!memcmp(input.text.bytes, "x", strlen("x")));
	TYPE(NULL, LIBTERMINPUT_BRACKETED_PASTE_END);
	TYPE("\033[200~x", LIBTERMINPUT_BRACKETED_PASTE_START);
	TYPE(NULL, LIBTERMINPUT_TEXT);
	TEST(input.text.nbytes == strlen("x"));
	TEST(!memcmp(input.text.bytes, "x", strlen("x")));
	TYPE("\033[201x~x\033[201~x", LIBTERMINPUT_TEXT);
	TEST(input.text.nbytes == strlen("\033[201x~x"));
	TEST(!memcmp(input.text.bytes, "\033[201x~x", strlen("\033[201x~x")));
	TYPE(NULL, LIBTERMINPUT_BRACKETED_PASTE_END);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE("\033[200~", LIBTERMINPUT_BRACKETED_PASTE_START);
	TYPE("\033[201~", LIBTERMINPUT_BRACKETED_PASTE_END);
	TYPE("\033[200~", LIBTERMINPUT_BRACKETED_PASTE_START);
	TYPE("\033[201",  LIBTERMINPUT_NONE);
	TYPE("x\033[20",  LIBTERMINPUT_TEXT);
	TEST(input.text.nbytes == strlen("\033[201x"));
	TEST(!memcmp(input.text.bytes, "\033[201x", strlen("\033[201x")));
	TYPE("1",  LIBTERMINPUT_NONE);
	TYPE("~",  LIBTERMINPUT_BRACKETED_PASTE_END);
	TYPE("\033[200~\033[201~", LIBTERMINPUT_BRACKETED_PASTE_START);
	TYPE(NULL, LIBTERMINPUT_BRACKETED_PASTE_END);
	TYPE("\033[200~\033[201", LIBTERMINPUT_BRACKETED_PASTE_START);
	TYPE("~", LIBTERMINPUT_BRACKETED_PASTE_END);

	TYPE("\033[200^", LIBTERMINPUT_NONE);
	TYPE("\033[200$", LIBTERMINPUT_NONE);
	TYPE("\033[200@", LIBTERMINPUT_NONE);
	TYPE("\033[201^", LIBTERMINPUT_NONE);
	TYPE("\033[201$", LIBTERMINPUT_NONE);
	TYPE("\033[201@", LIBTERMINPUT_NONE);
	TYPE("\033[n",    LIBTERMINPUT_NONE);
	TYPE("\033[0n",   LIBTERMINPUT_TERMINAL_IS_OK);
	TYPE("\033[3n",   LIBTERMINPUT_TERMINAL_IS_NOT_OK);
	libterminput_set_flags(&ctx, LIBTERMINPUT_AWAITING_CURSOR_POSITION);
	KEYPRESS_("\033[R", "", "", "", LIBTERMINPUT_F3, 0, 1);
	KEYPRESS_("\033[1R", "", "", "", LIBTERMINPUT_F3, 0, 1);
	TYPE("\033[1;1R", LIBTERMINPUT_CURSOR_POSITION);
	TEST(input.position.y == 1);
	TEST(input.position.x == 1);
	TYPE("\033[25;93R", LIBTERMINPUT_CURSOR_POSITION);
	TEST(input.position.y == 25);
	TEST(input.position.x == 93);
	libterminput_clear_flags(&ctx, LIBTERMINPUT_AWAITING_CURSOR_POSITION);

	TYPE("\033", LIBTERMINPUT_NONE);
	TYPE("\033", LIBTERMINPUT_NONE);
	TYPE("\033", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_ESC);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 3);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_ESC);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 2);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_ESC);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);

	TYPE("\033", LIBTERMINPUT_NONE);
	TYPE("\033", LIBTERMINPUT_NONE);
	TYPE("\033", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_ESC);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 3);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_ESC);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 2);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_ESC);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);

	KEYPRESS_SPECIAL_CHAR('\x7F', LIBTERMINPUT_ERASE);
	KEYPRESS_SPECIAL_CHAR('\b', LIBTERMINPUT_ERASE);
	KEYPRESS_SPECIAL_CHAR('\t', LIBTERMINPUT_TAB);
	KEYPRESS_SPECIAL_CHAR('\n', LIBTERMINPUT_ENTER);
	libterminput_set_flags(&ctx, LIBTERMINPUT_ESC_ON_BLOCK);
	KEYPRESS_SPECIAL_CHAR('\033', LIBTERMINPUT_ESC);
	libterminput_clear_flags(&ctx, LIBTERMINPUT_ESC_ON_BLOCK);

	TYPE("text", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TEST(input.keypress.symbol[0] == 't');
	TEST(input.keypress.symbol[1] == '\0');
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TEST(input.keypress.symbol[0] == 'e');
	TEST(input.keypress.symbol[1] == '\0');
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TEST(input.keypress.symbol[0] == 'x');
	TEST(input.keypress.symbol[1] == '\0');
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TEST(input.keypress.symbol[0] == 't');
	TEST(input.keypress.symbol[1] == '\0');

	TYPE("åäö", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "å"));
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "ä"));
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "ö"));

	TYPE("\033z", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_META);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "z"));
	TYPE("\033ö", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_META);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "ö"));

	buffer[0] = '-';
	buffer[0] |= 0x80;
	buffer[1] = 0;
	TYPE(buffer, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_META);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "-"));

	buffer[0] = 'Y';
	buffer[0] -= '@';
	buffer[1] = 0;
	TYPE(buffer, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_CTRL);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "Y"));

	buffer[0] = '\033';
	buffer[1] = 'Y';
	buffer[1] -= '@';
	buffer[2] = 0;
	TYPE(buffer, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_CTRL | LIBTERMINPUT_META);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "Y"));

	buffer[0] = 'Y';
	buffer[0] -= '@';
	buffer[0] |= 0x80;
	buffer[1] = 0;
	TYPE(buffer, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_CTRL | LIBTERMINPUT_META);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "Y"));

	buffer[0] = '\033';
	buffer[1] = 'Y';
	buffer[1] -= '@';
	buffer[1] |= 0x80;
	buffer[2] = 0;
	TYPE(buffer, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_CTRL | LIBTERMINPUT_META);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "Y"));

	buffer[0] = 0;
	TYPE_MEM(buffer, 1, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_CTRL);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, " "));

	buffer[0] = '\033';
	buffer[1] = 0;
	TYPE_MEM(buffer, 2, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_CTRL | LIBTERMINPUT_META);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, " "));

	for (i = 0; mice[i].str; i++)
		MOUSE(mice[i].str, mice[i].event, mice[i].button, mice[i].mods, mice[i].x, mice[i].y);

	MOUSE("\033[M !#",          LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,  0, 1, 3);
	MOUSE("\033[M!#!",          LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,  0, 3, 1);
	MOUSE("\033[M\"#!",         LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,  0, 3, 1);
	MOUSE("\033[M#!#",          LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,  0, 1, 3);
	MOUSE("\033[M\xa0!#" ,      LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, 0, 1, 3);
	MOUSE("\033[M\xa4!#",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, LIBTERMINPUT_SHIFT, 1, 3);
	MOUSE("\033[M\xa8!#",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, LIBTERMINPUT_META, 1, 3);
	MOUSE("\033[M\xac!#",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, LIBTERMINPUT_SHIFT | LIBTERMINPUT_META, 1, 3);
	MOUSE("\033[M\xb0!#",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, LIBTERMINPUT_CTRL, 1, 3);
	MOUSE("\033[M\xb1!#",       LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON2, LIBTERMINPUT_CTRL, 1, 3);
	MOUSE("\033[M   ",          LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,  0, 1, 1);

	MOUSE("\033[M\xdf\xff\xff", LIBTERMINPUT_MOTION,  11, 7, 255 - 32, 255 - 32);
	MOUSE("\033[M\x1f\x1f\x1f", LIBTERMINPUT_MOTION,  15, 7, 255, 255);

	TYPE("\033[M",   LIBTERMINPUT_NONE);
	MOUSE("!#!",     LIBTERMINPUT_PRESS, LIBTERMINPUT_BUTTON2, 0, 3, 1);
	TYPE("\033[M!",  LIBTERMINPUT_NONE);
	MOUSE("#!",      LIBTERMINPUT_PRESS, LIBTERMINPUT_BUTTON2, 0, 3, 1);
	TYPE("\033[M!#", LIBTERMINPUT_NONE);
	MOUSE("!",       LIBTERMINPUT_PRESS, LIBTERMINPUT_BUTTON2, 0, 3, 1);

	libterminput_set_flags(&ctx, LIBTERMINPUT_DECSET_1005);

	MOUSE("\033[M !#",          LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,  0, 1, 3);
	MOUSE("\033[M!#!",          LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON2,  0, 3, 1);
	MOUSE("\033[M\"#!",         LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON3,  0, 3, 1);
	MOUSE("\033[M#!#",          LIBTERMINPUT_RELEASE, LIBTERMINPUT_BUTTON1,  0, 1, 3);
	MOUSE("\033[M\xc2\xa0!#" ,  LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, 0, 1, 3);
	MOUSE("\033[M\xc2\xa4!#",   LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, LIBTERMINPUT_SHIFT, 1, 3);
	MOUSE("\033[M\xc2\xa8!#",   LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, LIBTERMINPUT_META, 1, 3);
	MOUSE("\033[M\xc2\xac!#",   LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, LIBTERMINPUT_SHIFT | LIBTERMINPUT_META, 1, 3);
	MOUSE("\033[M\xc2\xb0!#",   LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON1, LIBTERMINPUT_CTRL, 1, 3);
	MOUSE("\033[M\xc2\xb1!#",   LIBTERMINPUT_PRESS,   LIBTERMINPUT_XBUTTON2, LIBTERMINPUT_CTRL, 1, 3);
	MOUSE("\033[M   ",          LIBTERMINPUT_PRESS,   LIBTERMINPUT_BUTTON1,  0, 1, 1);

	TYPE("\033[M  \x1f", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_MACRO);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, " "));
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, " "));
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_SYMBOL);
	TEST(input.keypress.mods == LIBTERMINPUT_CTRL);
	TEST(input.keypress.times == 1);
	TEST(!strcmp(input.keypress.symbol, "_"));
	TYPE("\033[M \x1f ", LIBTERMINPUT_KEYPRESS);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE("\033[M\x1f  ", LIBTERMINPUT_KEYPRESS);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);

	MOUSE("\033[M \xc6\x89#",            LIBTERMINPUT_PRESS, LIBTERMINPUT_BUTTON1, 0, 361, 3);
	MOUSE("\033[M #\xc6\x89",            LIBTERMINPUT_PRESS, LIBTERMINPUT_BUTTON1, 0, 3, 361);
	MOUSE("\033[M \xc6\x89\xe0\xb4\xa5", LIBTERMINPUT_PRESS, LIBTERMINPUT_BUTTON1, 0, 361, 3333);

	TYPE("\033[M\xff", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_MACRO);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE("\033[M!\xff", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_MACRO);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE("\033[M!!\xff", LIBTERMINPUT_KEYPRESS);
	TEST(input.keypress.key == LIBTERMINPUT_MACRO);
	TEST(input.keypress.mods == 0);
	TEST(input.keypress.times == 1);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);
	TYPE(NULL, LIBTERMINPUT_KEYPRESS);

	TYPE("\033[M", LIBTERMINPUT_NONE);
	TYPE(" ",      LIBTERMINPUT_NONE);
	TYPE("\xc6",   LIBTERMINPUT_NONE);
	TYPE("\x89",   LIBTERMINPUT_NONE);
	TYPE("\xe0",   LIBTERMINPUT_NONE);
	TYPE("\xb4",   LIBTERMINPUT_NONE);
	MOUSE("\xa5",  LIBTERMINPUT_PRESS, LIBTERMINPUT_BUTTON1, 0, 361, 3333);

	libterminput_clear_flags(&ctx, LIBTERMINPUT_DECSET_1005);

	close(fds[1]);
	TEST(libterminput_read(fds[0], &input, &ctx) == 0);
	close(fds[0]);
	TEST(libterminput_read(fds[0], &input, &ctx) == -1 && errno == EBADF);
	return 0;
}
