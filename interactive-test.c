/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "libterminput.h"


int
main(void)
{
	struct libterminput_state ctx;
	union libterminput_input input;
	struct termios stty, saved_stty;
	int r;

	memset(&ctx, 0, sizeof(ctx));

	if (tcgetattr(STDIN_FILENO, &stty)) {
		perror("tcgetattr STDIN_FILENO");
		return 1;
	}
	saved_stty = stty;
	stty.c_lflag &= (tcflag_t)~(ECHO | ICANON);
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &stty)) {
		perror("tcsetattr STDIN_FILENO TCSAFLUSH");
		return 1;
	}

	while ((r = libterminput_read(STDIN_FILENO, &input, &ctx)) > 0) {
		if (input.type == LIBTERMINPUT_NONE) {
			printf("none\n");
		} else if (input.type == LIBTERMINPUT_KEYPRESS) {
			printf("keypress:\n");
			switch (input.keypress.key) {
			case LIBTERMINPUT_SYMBOL:
				printf("\t%s: %s\n", "key: symbol", input.keypress.symbol);
				break;
			case LIBTERMINPUT_UP:              printf("\t%s: %s\n", "key", "up");              break;
			case LIBTERMINPUT_DOWN:            printf("\t%s: %s\n", "key", "down");            break;
			case LIBTERMINPUT_RIGHT:           printf("\t%s: %s\n", "key", "right");           break;
			case LIBTERMINPUT_BEGIN:           printf("\t%s: %s\n", "key", "begin");           break;
			case LIBTERMINPUT_TAB:             printf("\t%s: %s\n", "key", "tab");             break;
			case LIBTERMINPUT_F1:              printf("\t%s: %s\n", "key", "f1");              break;
			case LIBTERMINPUT_F2:              printf("\t%s: %s\n", "key", "f2");              break;
			case LIBTERMINPUT_F3:              printf("\t%s: %s\n", "key", "f3");              break;
			case LIBTERMINPUT_F4:              printf("\t%s: %s\n", "key", "f4");              break;
			case LIBTERMINPUT_F5:              printf("\t%s: %s\n", "key", "f5");              break;
			case LIBTERMINPUT_F6:              printf("\t%s: %s\n", "key", "f6");              break;
			case LIBTERMINPUT_F7:              printf("\t%s: %s\n", "key", "f7");              break;
			case LIBTERMINPUT_F8:              printf("\t%s: %s\n", "key", "f8");              break;
			case LIBTERMINPUT_F9:              printf("\t%s: %s\n", "key", "f9");              break;
			case LIBTERMINPUT_F10:             printf("\t%s: %s\n", "key", "f10");             break;
			case LIBTERMINPUT_F11:             printf("\t%s: %s\n", "key", "f11");             break;
			case LIBTERMINPUT_F12:             printf("\t%s: %s\n", "key", "f12");             break;
			case LIBTERMINPUT_HOME:            printf("\t%s: %s\n", "key", "home");            break;
			case LIBTERMINPUT_INS:             printf("\t%s: %s\n", "key", "ins");             break;
			case LIBTERMINPUT_DEL:             printf("\t%s: %s\n", "key", "del");             break;
			case LIBTERMINPUT_END:             printf("\t%s: %s\n", "key", "end");             break;
			case LIBTERMINPUT_PRIOR:           printf("\t%s: %s\n", "key", "prior");           break;
			case LIBTERMINPUT_NEXT:            printf("\t%s: %s\n", "key", "next");            break;
			case LIBTERMINPUT_ERASE:           printf("\t%s: %s\n", "key", "erase");           break;
			case LIBTERMINPUT_ENTER:           printf("\t%s: %s\n", "key", "enter");           break;
			case LIBTERMINPUT_ESC:             printf("\t%s: %s\n", "key", "esc");             break;
			case LIBTERMINPUT_KEYPAD_0:        printf("\t%s: %s\n", "key", "keypad 0");        break;
			case LIBTERMINPUT_KEYPAD_1:        printf("\t%s: %s\n", "key", "keypad 1");        break;
			case LIBTERMINPUT_KEYPAD_2:        printf("\t%s: %s\n", "key", "keypad 2");        break;
			case LIBTERMINPUT_KEYPAD_3:        printf("\t%s: %s\n", "key", "keypad 3");        break;
			case LIBTERMINPUT_KEYPAD_4:        printf("\t%s: %s\n", "key", "keypad 4");        break;
			case LIBTERMINPUT_KEYPAD_5:        printf("\t%s: %s\n", "key", "keypad 5");        break;
			case LIBTERMINPUT_KEYPAD_6:        printf("\t%s: %s\n", "key", "keypad 6");        break;
			case LIBTERMINPUT_KEYPAD_7:        printf("\t%s: %s\n", "key", "keypad 7");        break;
			case LIBTERMINPUT_KEYPAD_8:        printf("\t%s: %s\n", "key", "keypad 8");        break;
			case LIBTERMINPUT_KEYPAD_9:        printf("\t%s: %s\n", "key", "keypad 9");        break;
			case LIBTERMINPUT_KEYPAD_PLUS:     printf("\t%s: %s\n", "key", "keypad plus");     break;
			case LIBTERMINPUT_KEYPAD_MINUS:    printf("\t%s: %s\n", "key", "keypad minus");    break;
			case LIBTERMINPUT_KEYPAD_TIMES:    printf("\t%s: %s\n", "key", "keypad times");    break;
			case LIBTERMINPUT_KEYPAD_DIVISION: printf("\t%s: %s\n", "key", "keypad division"); break;
			case LIBTERMINPUT_KEYPAD_DECIMAL:  printf("\t%s: %s\n", "key", "keypad decimal");  break;
			case LIBTERMINPUT_KEYPAD_COMMA:    printf("\t%s: %s\n", "key", "keypad comma");    break;
			case LIBTERMINPUT_KEYPAD_POINT:    printf("\t%s: %s\n", "key", "keypad point");    break;
			case LIBTERMINPUT_KEYPAD_ENTER:    printf("\t%s: %s\n", "key", "keypad enter");    break;
			default:
				printf("\t%s: %s\n", "key", "other");
				break;
			}
			printf("\t%s: %s\n", "shift", (input.keypress.mods & LIBTERMINPUT_SHIFT) ? "yes" : "no");
			printf("\t%s: %s\n", "meta",  (input.keypress.mods & LIBTERMINPUT_META)  ? "yes" : "no");
			printf("\t%s: %s\n", "ctrl",  (input.keypress.mods & LIBTERMINPUT_CTRL)  ? "yes" : "no");
			printf("\t%s: %s (%llu)\n", "will repeat", input.keypress.times > 1 ? "yes" : "no", input.keypress.times);
		} else if (input.type == LIBTERMINPUT_BRACKETED_PASTE_START) {
			printf("bracketed paste start\n");
		} else if (input.type == LIBTERMINPUT_BRACKETED_PASTE_END) {
			printf("bracketed paste end\n");
		} else if (input.type == LIBTERMINPUT_TEXT) {
			printf("text:\n");
			printf("\tlength: %zu\n", input.text.nbytes);
			printf("\tdata: %.512s\n", input.text.bytes);
		} else {
			printf("other\n");
		}
	}

	if (r < 0)
		perror("libterminput_read STDIN_FILENO");

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_stty);
	return -r;
}
