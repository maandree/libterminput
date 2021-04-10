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

	close(fds[1]);
	TEST(libterminput_read(fds[0], &input, &ctx) == 0);
	close(fds[0]);
	TEST(libterminput_read(fds[0], &input, &ctx) == -1 && errno == EBADF);
	return 0;
}
