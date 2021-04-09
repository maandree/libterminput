/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
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

	if (getenv("TEST_LIBTERMINPUT_DECSET_1005")) {
		fprintf(stderr, "LIBTERMINPUT_DECSET_1005 set\n");
		libterminput_set_flags(&ctx, LIBTERMINPUT_DECSET_1005);
	}

	if (tcgetattr(STDERR_FILENO, &stty)) {
		perror("tcgetattr STDERR_FILENO");
		return 1;
	}
	saved_stty = stty;
	stty.c_lflag &= (tcflag_t)~(ECHO | ICANON);
	if (tcsetattr(STDERR_FILENO, TCSAFLUSH, &stty)) {
		perror("tcsetattr STDERR_FILENO TCSAFLUSH");
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
		} else if (input.type == LIBTERMINPUT_MOUSEEVENT) {
			printf("mouseevent:\n");
			switch (input.mouseevent.event) {
			case LIBTERMINPUT_PRESS:             printf("\t%s: %s\n", "event", "press");             break;
			case LIBTERMINPUT_RELEASE:           printf("\t%s: %s\n", "event", "release");           break;
			case LIBTERMINPUT_MOTION:            printf("\t%s: %s\n", "event", "motion");            break;
			case LIBTERMINPUT_HIGHLIGHT_INSIDE:  printf("\t%s: %s\n", "event", "highlight inside");  goto was_highlight;
			case LIBTERMINPUT_HIGHLIGHT_OUTSIDE: printf("\t%s: %s\n", "event", "highlight outside"); goto was_highlight;
			default:
				printf("\t%s: %s\n", "event", "other");
				break;
			}
			switch (input.mouseevent.button) {
			case LIBTERMINPUT_NO_BUTTON:    printf("\t%s: %s\n", "button", "none");                             break;
			case LIBTERMINPUT_BUTTON1:      printf("\t%s: %s\n", "button", "button 1 (left)");                  break;
			case LIBTERMINPUT_BUTTON2:      printf("\t%s: %s\n", "button", "button 2 (middle)");                break;
			case LIBTERMINPUT_BUTTON3:      printf("\t%s: %s\n", "button", "button 3 (right)");                 break;
			case LIBTERMINPUT_SCROLL_UP:    printf("\t%s: %s\n", "button", "scroll up");                        break;
			case LIBTERMINPUT_SCROLL_DOWN:  printf("\t%s: %s\n", "button", "scroll down");                      break;
			case LIBTERMINPUT_SCROLL_LEFT:  printf("\t%s: %s\n", "button", "scroll left");                      break;
			case LIBTERMINPUT_SCROLL_RIGHT: printf("\t%s: %s\n", "button", "scroll right");                     break;
			case LIBTERMINPUT_XBUTTON1:     printf("\t%s: %s\n", "button", "extended button 1 (X1; backward)"); break;
			case LIBTERMINPUT_XBUTTON2:     printf("\t%s: %s\n", "button", "extended button 2 (X2; forward)");  break;
			case LIBTERMINPUT_XBUTTON3:     printf("\t%s: %s\n", "button", "extended button 3 (X3)");           break;
			case LIBTERMINPUT_XBUTTON4:     printf("\t%s: %s\n", "button", "extended button 4 (X4)");           break;
			default:
				printf("\t%s: %s\n", "button", "other");
				break;
			}
			printf("\t%s: %s\n", "shift", (input.mouseevent.mods & LIBTERMINPUT_SHIFT) ? "yes" : "no");
			printf("\t%s: %s\n", "meta",  (input.mouseevent.mods & LIBTERMINPUT_META)  ? "yes" : "no");
			printf("\t%s: %s\n", "ctrl",  (input.mouseevent.mods & LIBTERMINPUT_CTRL)  ? "yes" : "no");
		was_highlight:
			printf("\t%s: x=%zu, y=%zu\n", "position", input.mouseevent.x, input.mouseevent.y);
			if (LIBTERMINPUT_HIGHLIGHT_OUTSIDE) {
				printf("\t%s: x=%zu, y=%zu\n", "start", input.mouseevent.start_x, input.mouseevent.start_y);
				printf("\t%s: x=%zu, y=%zu\n", "end",   input.mouseevent.end_x,   input.mouseevent.end_y);
			}
			if (input.mouseevent.event == LIBTERMINPUT_PRESS) {
				printf("\033[1;4;4;10;10T");
				fflush(stdout);
			}
		} else {
			printf("other\n");
		}
	}

	if (r < 0)
		perror("libterminput_read STDIN_FILENO");

	tcsetattr(STDERR_FILENO, TCSAFLUSH, &saved_stty);
	return -r;
}
