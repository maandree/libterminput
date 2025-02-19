/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libterminput_parse_sequence__(union libterminput_input *input, struct libterminput_state *ctx)
{
	unsigned long long int *nums;
	size_t keylen, n, nnums = 0;
	char *p;

	/* Get number of numbers in the sequence, and allocate an array of at least 2 */
	if (ctx->key[0] == '[' && (ctx->key[1] == '<' ? isdigit(ctx->key[2]) : isdigit(ctx->key[1])))
		nnums += 1;
	for (n = 2U, p = ctx->key; *p; p++) {
		if (*p == ';') {
			n += 1U;
			nnums += 1U;
		}
	}
	nums = alloca(n * sizeof(*nums));
	nums[0] = nums[1] = 0;

	/* Read numbers and remove numbers and delimiters */
	for (keylen = 0, n = 0, p = ctx->key; *p; p++) {
		if (*p == ';') {
			nums[++n] = 0; /* We made sure above to allocate one extra */
		} else if (!isdigit(*p)) {
			ctx->key[keylen++] = *p;
		} else if (n < 3U) {
			if (nums[n] < (ULLONG_MAX - (*p & 15)) / 10)
				nums[n] = nums[n] * 10 + (*p & 15);
			else
				nums[n] = ULLONG_MAX;
		}
	}
	ctx->key[keylen] = '\0';

	/* Get times and mods, and reset symbol, and more as keypress */
	input->type = LIBTERMINPUT_KEYPRESS;
	input->keypress.symbol[0] = '\0';
	input->keypress.times = nums[0] + !nums[0];
	input->keypress.mods = nums[1] > 1 ? nums[1] - 1 : 0;
	input->keypress.mods |= ctx->meta > 1 ? LIBTERMINPUT_META : 0;

	switch (ctx->key[0]) {
	case '[':
		switch (keylen) {
		case 2:
			switch (ctx->key[1]) {
			case 'A': input->keypress.key = LIBTERMINPUT_UP;    break;
			case 'B': input->keypress.key = LIBTERMINPUT_DOWN;  break;
			case 'C': input->keypress.key = LIBTERMINPUT_RIGHT; break;
			case 'D': input->keypress.key = LIBTERMINPUT_LEFT;  break;
			case 'E': input->keypress.key = LIBTERMINPUT_BEGIN; break;
			case 'F': input->keypress.key = LIBTERMINPUT_END;   break;
			case 'G': input->keypress.key = LIBTERMINPUT_BEGIN; break;
			case 'H': input->keypress.key = LIBTERMINPUT_HOME;  break;
			case 'M':
				if (ctx->flags & LIBTERMINPUT_MACRO_ON_CSI_M)
					input->keypress.key = LIBTERMINPUT_MACRO;
				else
					libterminput_parse_csi_m_mouse_tracking__(input, ctx, nums, nnums);
				break;
			case 'P':
				input->keypress.key = LIBTERMINPUT_F1;
				if (ctx->flags & LIBTERMINPUT_PAUSE_ON_CSI_P)
					input->keypress.key = LIBTERMINPUT_PAUSE;
				break;
			case 'Q':
				input->keypress.key = LIBTERMINPUT_F2;
				break;
			case 'R':
				if ((ctx->flags & LIBTERMINPUT_AWAITING_CURSOR_POSITION) && nnums >= 2U) {
					input->position.type = LIBTERMINPUT_CURSOR_POSITION;
					input->position.y = (size_t)nums[0] + (size_t)!nums[0];
					input->position.x = (size_t)nums[1] + (size_t)!nums[1];
				} else {
					input->keypress.key = LIBTERMINPUT_F3;
				}
				break;
			case 'S':
				input->keypress.key = LIBTERMINPUT_F4;
				break;
			case 'T':
				libterminput_parse_csi_t_mouse_tracking__(input, ctx);
				break;
			case 'U': input->keypress.key = LIBTERMINPUT_NEXT;  break;
			case 'V': input->keypress.key = LIBTERMINPUT_PRIOR; break;
			case 'Z':
				if (ctx->flags & LIBTERMINPUT_SEPARATE_BACKTAB) {
					input->keypress.key = LIBTERMINPUT_BACKTAB;
				} else {
					input->keypress.key = LIBTERMINPUT_TAB;
					input->keypress.mods |= LIBTERMINPUT_SHIFT;
				}
				break;
			case 'a': input->keypress.key = LIBTERMINPUT_UP;    goto shift;
			case 'b': input->keypress.key = LIBTERMINPUT_DOWN;  goto shift;
			case 'c': input->keypress.key = LIBTERMINPUT_RIGHT; goto shift;
			case 'd': input->keypress.key = LIBTERMINPUT_LEFT;  goto shift;
			case 'n':
				if (nnums == 1U && nums[0] == 0)
					input->type = LIBTERMINPUT_TERMINAL_IS_OK;
				else if (nnums == 1U && nums[0] == 3)
					input->type = LIBTERMINPUT_TERMINAL_IS_NOT_OK;
				else
					goto suppress;
				break;
			case 't':
				libterminput_parse_csi_small_t_mouse_tracking__(input, ctx);
				break;
			case 'u':
				if (nums[0] > 0x10FFFFULL || (nums[0] & 0xFFF800ULL) == 0xD800ULL) {
					input->type = LIBTERMINPUT_NONE;
					break;
				}
				libterminput_encode_utf8__(nums[0], input->keypress.symbol);
				input->keypress.times = 1;
				break;
			case '$':
				input->keypress.mods |= LIBTERMINPUT_SHIFT;
				if (nums[0] >= 200)
					goto suppress;
				goto tilde_case;
			case '@':
				if (ctx->flags & LIBTERMINPUT_INS_ON_CSI_AT) {
					input->keypress.key = LIBTERMINPUT_INS;
					break;
				}
				input->keypress.mods |= LIBTERMINPUT_SHIFT;
				/* fall through */
			case '^':
				input->keypress.mods |= LIBTERMINPUT_CTRL;
				if (nums[0] >= 200)
					goto suppress;
				/* fall through */
			case '~':
			tilde_case:
				input->keypress.times = 1;
				switch (nums[0]) {
				case  1: input->keypress.key = LIBTERMINPUT_HOME;  break;
				case  2: input->keypress.key = LIBTERMINPUT_INS;   break;
				case  3: input->keypress.key = LIBTERMINPUT_DEL;   break;
				case  4: input->keypress.key = LIBTERMINPUT_END;   break;
				case  5: input->keypress.key = LIBTERMINPUT_PRIOR; break;
				case  6: input->keypress.key = LIBTERMINPUT_NEXT;  break;
				case  7: input->keypress.key = LIBTERMINPUT_HOME;  break;
				case  8: input->keypress.key = LIBTERMINPUT_END;   break;
				case  9: input->keypress.key = LIBTERMINPUT_ESC;   break; /* just made this one up */
				case 11: input->keypress.key = LIBTERMINPUT_F1;    break;
				case 12: input->keypress.key = LIBTERMINPUT_F2;    break;
				case 13: input->keypress.key = LIBTERMINPUT_F3;    break;
				case 14: input->keypress.key = LIBTERMINPUT_F4;    break;
				case 15: input->keypress.key = LIBTERMINPUT_F5;    break;
				case 17: input->keypress.key = LIBTERMINPUT_F6;    break;
				case 18: input->keypress.key = LIBTERMINPUT_F7;    break;
				case 19: input->keypress.key = LIBTERMINPUT_F8;    break;
				case 20: input->keypress.key = LIBTERMINPUT_F9;    break;
				case 21: input->keypress.key = LIBTERMINPUT_F10;   break;
				case 23: input->keypress.key = LIBTERMINPUT_F11;   break;
				case 24: input->keypress.key = LIBTERMINPUT_F12;   break;
				case 25: input->keypress.key = LIBTERMINPUT_F1;    goto shift;
				case 26: input->keypress.key = LIBTERMINPUT_F2;    goto shift;
				case 28: input->keypress.key = LIBTERMINPUT_F3;    goto shift;
				case 29: input->keypress.key = LIBTERMINPUT_F4;    goto shift;
				case 31: input->keypress.key = LIBTERMINPUT_F5;    goto shift;
				case 32: input->keypress.key = LIBTERMINPUT_F6;    goto shift;
				case 33: input->keypress.key = LIBTERMINPUT_F7;    goto shift;
				case 34: input->keypress.key = LIBTERMINPUT_F8;    goto shift;
				case 200:
					ctx->bracketed_paste = 1;
					input->type = LIBTERMINPUT_BRACKETED_PASTE_START;
					return;
				case 201:
					ctx->bracketed_paste = 0;
					input->type = LIBTERMINPUT_BRACKETED_PASTE_END;
					return;
				default:
					goto suppress;
				}
				break;
			default:
				goto suppress;
			}
			break;

		case 3:
			switch (ctx->key[1] == '[' ? ctx->key[2] : 0) {
			case 'A': input->keypress.key = LIBTERMINPUT_F1; break;
			case 'B': input->keypress.key = LIBTERMINPUT_F2; break;
			case 'C': input->keypress.key = LIBTERMINPUT_F3; break;
			case 'D': input->keypress.key = LIBTERMINPUT_F4; break;
			case 'E': input->keypress.key = LIBTERMINPUT_F5; break;
			default:
				if (ctx->key[1] == '<' && (ctx->key[2] == 'M' || ctx->key[2] == 'm') && nnums >= 3U) {
					/* Parsing for \e[?1003;1006h output. */
					input->mouseevent.event = LIBTERMINPUT_PRESS;
					if (ctx->key[2] == 'm')
						input->mouseevent.event = LIBTERMINPUT_RELEASE;
					libterminput_parse_decimal_mouse_tracking__(input, nums);
				} else {
					goto suppress;
				}
			}
			break;

		default:
			goto suppress;
		}
		break;

	case '?': /* '?' is attested as synonym for 'O' when the next character is in "pqrstuvwxymlnM" */
	case 'O':
		switch (!ctx->key[2] ? ctx->key[1] : 0) {
		case 'A': input->keypress.key = LIBTERMINPUT_UP;              break;
		case 'B': input->keypress.key = LIBTERMINPUT_DOWN;            break;
		case 'C': input->keypress.key = LIBTERMINPUT_RIGHT;           break;
		case 'D': input->keypress.key = LIBTERMINPUT_LEFT;            break;
		case 'E': input->keypress.key = LIBTERMINPUT_BEGIN;           break; /* not attested */
		case 'F': input->keypress.key = LIBTERMINPUT_END;             break;
		case 'G': input->keypress.key = LIBTERMINPUT_BEGIN;           break; /* not attested */
		case 'H': input->keypress.key = LIBTERMINPUT_HOME;            break;
		case 'I': input->keypress.key = LIBTERMINPUT_F12;             break;
		case 'J': input->keypress.key = LIBTERMINPUT_F1;              goto shift;
		case 'K': input->keypress.key = LIBTERMINPUT_F2;              goto shift;
		case 'L': input->keypress.key = LIBTERMINPUT_F3;              goto shift;
		case 'M': input->keypress.key = LIBTERMINPUT_KEYPAD_ENTER;    break;
		case 'N': input->keypress.key = LIBTERMINPUT_F4;              goto shift;
		case 'P': input->keypress.key = LIBTERMINPUT_F1;              break;
		case 'Q': input->keypress.key = LIBTERMINPUT_F2;              break;
		case 'R': input->keypress.key = LIBTERMINPUT_F3;              break;
		case 'S': input->keypress.key = LIBTERMINPUT_F4;              break;
		case 'T': input->keypress.key = LIBTERMINPUT_F5;              break;
		case 'U': input->keypress.key = LIBTERMINPUT_F6;              break;
		case 'V': input->keypress.key = LIBTERMINPUT_F7;              break; /* not attested */
		case 'W': input->keypress.key = LIBTERMINPUT_F8;              break; /* not attested */
		case 'X': input->keypress.key = LIBTERMINPUT_F9;              break; /* not attested */
		case 'Y': input->keypress.key = LIBTERMINPUT_F10;             break;
		case 'Z': input->keypress.key = LIBTERMINPUT_F11;             break; /* not attested */
		case 'b': input->keypress.key = LIBTERMINPUT_KEYPAD_POINT;    break;
		case 'e': input->keypress.key = LIBTERMINPUT_F7;              break;
		case 'f': input->keypress.key = LIBTERMINPUT_F8;              break;
		case 'j': input->keypress.key = LIBTERMINPUT_KEYPAD_TIMES;    break;
		case 'k': input->keypress.key = LIBTERMINPUT_KEYPAD_PLUS;     break;
		case 'l': input->keypress.key = LIBTERMINPUT_KEYPAD_COMMA;    break;
		case 'm': input->keypress.key = LIBTERMINPUT_KEYPAD_MINUS;    break;
		case 'n': input->keypress.key = LIBTERMINPUT_KEYPAD_DECIMAL;  break;
		case 'o': input->keypress.key = LIBTERMINPUT_KEYPAD_DIVISION; break;
		case 'p': input->keypress.key = LIBTERMINPUT_KEYPAD_0;        break;
		case 'q': input->keypress.key = LIBTERMINPUT_KEYPAD_1;        break;
		case 'r': input->keypress.key = LIBTERMINPUT_KEYPAD_2;        break;
		case 's': input->keypress.key = LIBTERMINPUT_KEYPAD_3;        break;
		case 't': input->keypress.key = LIBTERMINPUT_KEYPAD_4;        break;
		case 'u': input->keypress.key = LIBTERMINPUT_KEYPAD_5;        break;
		case 'v': input->keypress.key = LIBTERMINPUT_KEYPAD_6;        break;
		case 'w': input->keypress.key = LIBTERMINPUT_KEYPAD_7;        break;
		case 'x': input->keypress.key = LIBTERMINPUT_KEYPAD_8;        break;
		case 'y': input->keypress.key = LIBTERMINPUT_KEYPAD_9;        break;
		default:
			goto suppress;
		}
		break;
	shift:
		input->keypress.mods |= LIBTERMINPUT_SHIFT;
		break;

	default:
		/* This shouldn't happen (without goto) */
	suppress:
		input->type = LIBTERMINPUT_NONE;
		break;
	}
}
