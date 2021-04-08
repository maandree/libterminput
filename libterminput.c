/* See LICENSE file for copyright and license details. */
#include "libterminput.h"

#include <alloca.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>


struct input {
	enum libterminput_mod mods;
	char symbol[7];
};


static int
read_input(int fd, struct input *input, struct libterminput_state *ctx)
{
	unsigned char c, tc;
	int r;

	/* Get next byte from input */
	if (ctx->stored_head != ctx->stored_tail) {
		c = ((unsigned char *)ctx->stored)[ctx->stored_tail++];
		if (ctx->stored_tail == ctx->stored_head)
			ctx->stored_tail = ctx->stored_head = 0;
	} else {
		r = read(fd, &c, 1);
		if (r <= 0)
			return r;
	}

again:
	if (ctx->n) {
		/* Continuation of multibyte-character */
		if ((c & 0xC0) != 0x80) {
			/* Short multibyte-character: return short and store read byte from next input */
			input->mods = ctx->mods;
			ctx->partial[ctx->npartial] = '\0';
			ctx->n = 0;
			ctx->npartial = 0;
			ctx->mods = 0;
			ctx->stored[ctx->stored_head++] = c;
			strcpy(input->symbol, ctx->partial);
			return 1;
		} else {
			/* Store byte, and if done, return */
			ctx->partial[ctx->npartial++] = c;
			if (ctx->npartial == ctx->n) {
				ctx->partial[ctx->npartial] = '\0';
				input->mods = ctx->mods;
				ctx->npartial = 0;
				ctx->mods = 0;
				ctx->n = 0;
				strcpy(input->symbol, ctx->partial);
				return 1;
			}
		}
	} else if (c == 033 && !*ctx->key) {
		/* ESC at the beginning, save as a Meta/ESC */
		ctx->meta += 1;
	} else if (c == 0) {
		/* CTRL on Space */
		input->symbol[0] = ' ';
		input->symbol[1] = '\0';
		input->mods = ctx->mods | LIBTERMINPUT_CTRL;
		ctx->mods = 0;
		return 1;
	} else if (c < (unsigned char)' ' && (char)c != '\t' && (char)c != '\b' && (char)c != '\n') {
		/* CTRL on some some character key */
		input->symbol[0] = (char)c + '@';
		input->symbol[1] = '\0';
		input->mods = ctx->mods | LIBTERMINPUT_CTRL;
		ctx->mods = 0;
		return 1;
	} else if ((c & 0xC0) == 0xC0) {
		/* Beginning of multibyte-character */
		ctx->n = 0;
		for (tc = c; tc & 0x80; tc <<= 1)
			ctx->n++;
		if (ctx->n > 6) {
			/* If overlong, return first byte a single-byte-character */
			input->symbol[0] = c;
			input->symbol[1] = '\0';
			input->mods = ctx->mods;
			ctx->mods = 0;
			return 1;
		}
		ctx->partial[0] = c;
		ctx->npartial = 1;
	} else if (c & 0x80) {
		/* 8th bit set to signify META */
		c ^= 0x80;
		ctx->mods |= LIBTERMINPUT_META;
		if (c == 033)
			goto single_byte;
		goto again;
	} else {
	single_byte:
		/* Single-byte-character */
		input->symbol[0] = c;
		input->symbol[1] = '\0';
		input->mods = ctx->mods;
		ctx->mods = 0;
		return 1;
	}

	input->symbol[0] = '\0';
	input->mods = -1;
	return 1;
}


static void
encode_utf8(unsigned long long int codepoint, char buffer[7])
{
	static const char masks[6] = {0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
	static const unsigned long long int limits[6] = {
		1ULL << (7 + 0 * 6),
		1ULL << (5 + 1 * 6),
		1ULL << (4 + 2 * 6),
		1ULL << (3 + 3 * 6),
		1ULL << (2 + 4 * 6),
		1ULL << (1 + 5 * 6)
	};
	size_t len;
	for (len = 0; codepoint >= limits[len]; len++);
	buffer[0] = masks[len];
	len += 1;
	buffer[len] = '\0';
	for (; --len; codepoint >>= 6)
		buffer[len] = (char)(codepoint & 0x3FULL);
	buffer[0] |= (char)codepoint;
}


static void
parse_sequence(union libterminput_input *input, struct libterminput_state *ctx)
{
	unsigned long long int *nums;
	size_t keylen, n;
	char *p;

	/* Get number of numbers in the sequence, and allocate an array of at least 2 */
	for (n = 2, p = ctx->key; *p; p++)
		n += *p == ';';
	nums = alloca(n * sizeof(*nums));
	nums[0] = nums[1] = 0;

	/* Read numbers and remove numbers and delimiters */
	for (keylen = 0, n = 0, p = ctx->key; *p; p++) {
		if (*p == ';') {
			nums[++n] = 0; /* We made sure above to allocate one extra */
		} else if (!isdigit(*p)) {
			ctx->key[keylen++] = *p;
		} else if (n < 3) {
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
			case '@': input->keypress.key = LIBTERMINPUT_INS;   break;
			case 'A': input->keypress.key = LIBTERMINPUT_UP;    break;
			case 'B': input->keypress.key = LIBTERMINPUT_DOWN;  break;
			case 'C': input->keypress.key = LIBTERMINPUT_RIGHT; break;
			case 'D': input->keypress.key = LIBTERMINPUT_LEFT;  break;
			case 'E': input->keypress.key = LIBTERMINPUT_BEGIN; break;
			case 'F': input->keypress.key = LIBTERMINPUT_END;   break;
			case 'G': input->keypress.key = LIBTERMINPUT_BEGIN; break;
			case 'H': input->keypress.key = LIBTERMINPUT_HOME;  break;
			case 'M': input->keypress.key = LIBTERMINPUT_MACRO; break;
			case 'P': input->keypress.key = LIBTERMINPUT_PAUSE; break;
			case 'U': input->keypress.key = LIBTERMINPUT_NEXT;  break;
			case 'V': input->keypress.key = LIBTERMINPUT_PRIOR; break;
			case 'Z':
				input->keypress.key = LIBTERMINPUT_TAB;
				input->keypress.mods |= LIBTERMINPUT_SHIFT;
				break;
			case 'u':
				if (nums[0] > 0x10FFFFULL) {
					input->type = LIBTERMINPUT_NONE;
					break;
				}
				encode_utf8(nums[0], input->keypress.symbol);
				input->keypress.times = 1;
				break;
			case '~':
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
				case 25: input->keypress.key = LIBTERMINPUT_F1;    break;
				case 26: input->keypress.key = LIBTERMINPUT_F2;    break;
				case 28: input->keypress.key = LIBTERMINPUT_F3;    break;
				case 29: input->keypress.key = LIBTERMINPUT_F4;    break;
				case 31: input->keypress.key = LIBTERMINPUT_F5;    break;
				case 32: input->keypress.key = LIBTERMINPUT_F6;    break;
				case 33: input->keypress.key = LIBTERMINPUT_F7;    break;
				case 34: input->keypress.key = LIBTERMINPUT_F8;    break;
				case 200:
					ctx->bracketed_paste = 1;
					input->type = LIBTERMINPUT_BRACKETED_PASTE_START;
					return;
				case 201:
					ctx->bracketed_paste = 0;
					input->type = LIBTERMINPUT_BRACKETED_PASTE_END;
					return;
				default:
					input->type = LIBTERMINPUT_NONE;
					return;
				}
				if (25 <= nums[0] && nums[0] <= 34)
					input->keypress.mods |= LIBTERMINPUT_SHIFT;
				break;
			default:
				input->type = LIBTERMINPUT_NONE;
				break;
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
				input->type = LIBTERMINPUT_NONE;
				break;
			}
			break;
		default:
			input->type = LIBTERMINPUT_NONE;
			break;
		}
		break;

	case 'O':
		switch (!ctx->key[2] ? ctx->key[1] : 0) {
		case 'A': input->keypress.key = LIBTERMINPUT_UP;           break;
		case 'B': input->keypress.key = LIBTERMINPUT_DOWN;         break;
		case 'C': input->keypress.key = LIBTERMINPUT_RIGHT;        break;
		case 'D': input->keypress.key = LIBTERMINPUT_LEFT;         break;
		case 'H': input->keypress.key = LIBTERMINPUT_HOME;         break;
		case 'F': input->keypress.key = LIBTERMINPUT_END;          break;
		case 'P': input->keypress.key = LIBTERMINPUT_F1;           break;
		case 'Q': input->keypress.key = LIBTERMINPUT_F2;           break;
		case 'R': input->keypress.key = LIBTERMINPUT_F3;           break;
		case 'S': input->keypress.key = LIBTERMINPUT_F4;           break;
		case 'p': input->keypress.key = LIBTERMINPUT_KEYPAD_0;     break;
		case 'q': input->keypress.key = LIBTERMINPUT_KEYPAD_1;     break;
		case 'r': input->keypress.key = LIBTERMINPUT_KEYPAD_2;     break;
		case 's': input->keypress.key = LIBTERMINPUT_KEYPAD_3;     break;
		case 't': input->keypress.key = LIBTERMINPUT_KEYPAD_4;     break;
		case 'u': input->keypress.key = LIBTERMINPUT_KEYPAD_5;     break;
		case 'v': input->keypress.key = LIBTERMINPUT_KEYPAD_6;     break;
		case 'w': input->keypress.key = LIBTERMINPUT_KEYPAD_7;     break;
		case 'x': input->keypress.key = LIBTERMINPUT_KEYPAD_8;     break;
		case 'y': input->keypress.key = LIBTERMINPUT_KEYPAD_9;     break;
		case 'm': input->keypress.key = LIBTERMINPUT_KEYPAD_MINUS; break;
		case 'l': input->keypress.key = LIBTERMINPUT_KEYPAD_COMMA; break;
		case 'b': input->keypress.key = LIBTERMINPUT_KEYPAD_POINT; break;
		case 'M': input->keypress.key = LIBTERMINPUT_KEYPAD_ENTER; break;
		default:
			input->type = LIBTERMINPUT_NONE;
			break;
		}
		break;

	default:
		/* This shouldn't happen */
		input->type = LIBTERMINPUT_NONE;
		break;
	}		
}


static int
read_bracketed_paste(int fd, union libterminput_input *input, struct libterminput_state *ctx)
{
	ssize_t r;
	size_t n;

	/* Unfortunately there is no standard for how to handle pasted ESC's,
	 * not even ESC [201~ or ESC ESC. Terminates seem to just paste ESC as
	 * is, so we cannot do anything about them, however, a good terminal
	 * would stop the paste at the ~ in ESC [201~, send ~ as normal, and
	 * then continue the brackated paste mode. */

	if (ctx->stored_head - ctx->stored_tail) {
		for (n = ctx->stored_tail; n + 6 < ctx->stored_head; n++) {
			if (ctx->stored[n + 0] == '\033' && ctx->stored[n + 1] == '[' && ctx->stored[n + 2] == '2' &&
			    ctx->stored[n + 3] == '0'    && ctx->stored[n + 4] == '0' && ctx->stored[n + 5] == '~')
				break;
		}
		if (n == ctx->stored_tail && ctx->stored_head - ctx->stored_tail >= 6) {
			ctx->stored_tail += 6;
			if (ctx->stored_tail == ctx->stored_head)
				ctx->stored_tail = ctx->stored_head = 0;
			input->type = LIBTERMINPUT_BRACKETED_PASTE_END;
			return 0;
		}
		input->text.nbytes = ctx->stored_head - ctx->stored_tail;
		input->text.type = LIBTERMINPUT_TEXT;
		memcpy(input->text.bytes, &ctx->stored[ctx->stored_tail], n - ctx->stored_tail);
		ctx->stored_tail = n;
		if (ctx->stored_tail == ctx->stored_head)
			ctx->stored_tail = ctx->stored_head = 0;
		return 0;
	}

	r = read(fd, input->text.bytes, sizeof(input->text.bytes));
	if (r <= 0)
		return (int)r;

	input->text.nbytes = (size_t)r;
	for (n = 0; n + 6 < input->text.nbytes; n++) {
		if (input->text.bytes[n + 0] == '\033' && input->text.bytes[n + 1] == '[' && input->text.bytes[n + 2] == '2' &&
		    input->text.bytes[n + 3] == '0'    && input->text.bytes[n + 4] == '0' && input->text.bytes[n + 5] == '~')
			break;
	}
	if (!n && input->text.nbytes >= 6) {
		ctx->stored_tail = 0;
		ctx->stored_head = input->text.nbytes - 6;
		memcpy(ctx->stored, &input->text.bytes[6], ctx->stored_head);
		if (ctx->stored_tail == ctx->stored_head)
			ctx->stored_tail = ctx->stored_head = 0;
		input->type = LIBTERMINPUT_BRACKETED_PASTE_END;
		return 0;
	}
	ctx->stored_tail = 0;
	ctx->stored_head = input->text.nbytes - n;
	input->text.nbytes = n;
	input->text.type = LIBTERMINPUT_TEXT;
	return 0;
}


int
libterminput_read(int fd, union libterminput_input *input, struct libterminput_state *ctx)
{
	struct input ret;
	size_t n, m;
	char *p;
	int r;

	if (!ctx->inited) {
		ctx->inited = 1;
		memset(input, 0, sizeof(*input));
	} else if (input->type == LIBTERMINPUT_KEYPRESS && input->keypress.times > 1) {
		input->keypress.times -= 1;
		return 1;
	}

	if (ctx->bracketed_paste)
		return read_bracketed_paste(fd, input, ctx);

	r = read_input(fd, &ret, ctx);
	if (r <= 0)
		return r;

again:
	if (!*ret.symbol) {
		/* Incomplete input */
		if (ctx->meta < 3) {
			/* Up to two Meta/ESC, wait until a third or something else is read */
			input->type = LIBTERMINPUT_NONE;
			return 1;
		}
		/* Three ESC's */
		input->type = LIBTERMINPUT_KEYPRESS;
		input->keypress.key = LIBTERMINPUT_ESC;
		input->keypress.times = 3;
		input->keypress.mods = ret.mods;
		input->keypress.symbol[0] = '\0';
		ctx->meta -= 3;
	} else if (*ctx->key) {
		/* Special keys */
		if (ret.mods) {
			/* Special key was aborted, restart */
			*ctx->key = '\0';
			goto again;
		}
		/* Add new input to sequence */
		n = strlen(ctx->key);
		m = strlen(ret.symbol);
		if (n + m >= sizeof(ctx->key)) {
			/* Abort if too long */
			input->type = LIBTERMINPUT_NONE;
			return 1;
		}
		p = stpcpy(&ctx->key[n], ret.symbol);
		/* Check if sequence is complete */
		if (!isalpha(p[-1]) && p[-1] != '~') {
			input->type = LIBTERMINPUT_NONE;
			return 1;
		}
		/* Parse the complete sequence */
		parse_sequence(input, ctx);
		/* Reset */
		ctx->meta = 0;
		ctx->key[0] = '\0';
	} else if (ctx->meta && (!strcmp(ret.symbol, "[") || !strcmp(ret.symbol, "O"))) {
		/* ESC [ or ESC 0 is used as the beginning of most special keys */
		strcpy(ctx->key, ret.symbol);
		input->type == LIBTERMINPUT_NONE;
	} else {
		/* Character input and single-byte special keys */
		input->type = LIBTERMINPUT_KEYPRESS;
		input->keypress.mods = ret.mods;
		input->keypress.times = 1;
		if (ctx->meta) {
			/* Transfer meta modifier from state to input */
			input->keypress.mods |= LIBTERMINPUT_META;
			ctx->meta = 0;
		}
		switch (ret.symbol[1] ? 0 : ret.symbol[0]) {
		case 127:
		case '\b':
			input->keypress.key = LIBTERMINPUT_ERASE;
			input->keypress.symbol[0] = '\0';
			break;
		case '\t':
			input->keypress.key = LIBTERMINPUT_TAB;
			input->keypress.symbol[0] = '\0';
			break;
		case '\n':
			input->keypress.key = LIBTERMINPUT_ENTER;
			input->keypress.symbol[0] = '\0';
			break;
		default:
			input->keypress.key = LIBTERMINPUT_SYMBOL;
			strcpy(input->keypress.symbol, ret.symbol);
			break;
		}
	}

	return 1;
}
