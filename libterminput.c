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
	ssize_t r;

	/* Get next byte from input */
	if (ctx->stored_head != ctx->stored_tail) {
		c = ((unsigned char *)ctx->stored)[ctx->stored_tail++];
		if (ctx->stored_tail == ctx->stored_head)
			ctx->stored_tail = ctx->stored_head = 0;
	} else {
		r = read(fd, ctx->stored, sizeof(ctx->stored));
		if (r <= 0)
			return (int)r;
		c = (unsigned char)ctx->stored[0];
		if (r > 1) {
			ctx->stored_tail = 1;
			ctx->stored_head = (size_t)r;
		}
	}

again:
	if (ctx->n) {
		/* Continuation of multibyte-character */
		if ((c & 0xC0) != 0x80) {
			/* Short multibyte-character: return short and store read byte from next input */
			input->mods = ctx->mods;
			ctx->partial[(unsigned char)ctx->npartial] = '\0';
			ctx->n = 0;
			ctx->npartial = 0;
			ctx->mods = 0;
			ctx->stored[ctx->stored_head++] = (char)c;
			strcpy(input->symbol, ctx->partial);
			return 1;
		} else {
			/* Store byte, and if done, return */
			ctx->partial[(unsigned char)ctx->npartial++] = (char)c;
			if (ctx->npartial == ctx->n) {
				ctx->partial[(unsigned char)ctx->npartial] = '\0';
				input->mods = ctx->mods;
				ctx->npartial = 0;
				ctx->mods = 0;
				ctx->n = 0;
				strcpy(input->symbol, ctx->partial);
				return 1;
			}
		}
	} else if (c == 033 && !*ctx->key) {
		/* ESC at the beginning, save as a Meta/ESC (for default behaviour) */
		if ((ctx->flags & LIBTERMINPUT_ESC_ON_BLOCK) && ctx->stored_tail == ctx->stored_head) {
			input->symbol[0] = (char)c;
			input->symbol[1] = '\0';
			input->mods = ctx->mods;
			ctx->mods = 0;
			return 1;
		}
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
			input->symbol[0] = (char)c;
			input->symbol[1] = '\0';
			input->mods = ctx->mods;
			ctx->mods = 0;
			return 1;
		}
		ctx->partial[0] = (char)c;
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
		input->symbol[0] = (char)c;
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
	static const char masks[6] = {(char)0x00, (char)0xC0, (char)0xE0, (char)0xF0, (char)0xF8, (char)0xFC};
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


static int
check_utf8_char(const char *s, size_t *lenp, size_t size)
{
	*lenp = 0;
	if (!size) {
		return 0;
	} else if ((*s & 0x80) == 0) {
		*lenp = 1;
		return 1;
	} else if ((*s & 0xE0) == 0xC0) {
		goto need_2;
	} else if ((*s & 0xF0) == 0xE0) {
		goto need_3;
	} else if ((*s & 0xF8) == 0xF0) {
		goto need_4;
	} else if ((*s & 0xFC) == 0xF8) {
		goto need_5;
	} else if ((*s & 0xFE) == 0xFC) {
		goto need_6;
	} else {
		*lenp = 0;
		return -1;
	}

need_6:
	if (!size--) return 0;
	if ((s[5] & 0xC0) != 0x80) return -1;
	++*lenp;

need_5:
	if (!size--) return 0;
	if ((s[4] & 0xC0) != 0x80) return -1;
	++*lenp;

need_4:
	if (!size--) return 0;
	if ((s[3] & 0xC0) != 0x80) return -1;
	++*lenp;

need_3:
	if (!size--) return 0;
	if ((s[2] & 0xC0) != 0x80) return -1;
	++*lenp;

need_2:
	if (!size--) return 0;
	if ((s[1] & 0xC0) != 0x80) return -1;
	++*lenp;

	if (!size--) return 0;
	++*lenp;
	return 1;
}


static unsigned long long int
utf8_decode(const char *s, size_t *ip)
{
	unsigned long long int cp = 0;
	size_t len;

	if ((s[*ip] & 0x80) == 0) {
		return (unsigned long long int)s[(*ip)++];
	} else if ((s[*ip] & 0xE0) == 0xC0) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xC0U);
		len = 1;
		goto need_1;
	} else if ((s[*ip] & 0xF0) == 0xE0) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xE0U);
		len = 2;
		goto need_2;
	} else if ((s[*ip] & 0xF8) == 0xF0) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xF0U);
		len = 3;
		goto need_3;
	} else if ((s[*ip] & 0xFC) == 0xF8) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xF8U);
		len = 4;
		goto need_4;
	} else if ((s[*ip] & 0xFE) == 0xFC) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xFCU);
		len = 5;
		goto need_5;
	}

need_5:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

need_4:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

need_3:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

need_2:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

need_1:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

	/* Let's ignore the 0x10FFFF upper bound. */

	if (cp < 1ULL << (7 + 0 * 6))
		return len > 1 ? 0ULL : cp;
	if (cp < 1ULL << (5 + 1 * 6))
		return len > 1 ? 0ULL : cp;
	if (cp < 1ULL << (4 + 2 * 6))
		return len > 1 ? 0ULL : cp;
	if (cp < 1ULL << (3 + 3 * 6))
		return len > 1 ? 0ULL : cp;
	if (cp < 1ULL << (2 + 4 * 6))
		return len > 1 ? 0ULL : cp;
	if (cp < 1ULL << (1 + 5 * 6))
		return len > 1 ? 0ULL : cp;

	return 0;
}


static void
parse_sequence(union libterminput_input *input, struct libterminput_state *ctx)
{
	unsigned long long int *nums, numsbuf[6];
	size_t keylen, n, nnums = 0, pos;
	char *p;

	/* Get number of numbers in the sequence, and allocate an array of at least 2 */
	if (ctx->key[0] == '[' && (ctx->key[1] == '<' ? isdigit(ctx->key[2]) : isdigit(ctx->key[1])))
		nnums += 1;
	for (n = 2, p = ctx->key; *p; p++) {
		if (*p == ';') {
			n += 1;
			nnums += 1;
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
			case 'A': input->keypress.key = LIBTERMINPUT_UP;    break;
			case 'B': input->keypress.key = LIBTERMINPUT_DOWN;  break;
			case 'C': input->keypress.key = LIBTERMINPUT_RIGHT; break;
			case 'D': input->keypress.key = LIBTERMINPUT_LEFT;  break;
			case 'E': input->keypress.key = LIBTERMINPUT_BEGIN; break;
			case 'F': input->keypress.key = LIBTERMINPUT_END;   break;
			case 'G': input->keypress.key = LIBTERMINPUT_BEGIN; break;
			case 'H': input->keypress.key = LIBTERMINPUT_HOME;  break;
			case 'M':
				if (ctx->flags & LIBTERMINPUT_MACRO_ON_CSI_M) {
					input->keypress.key = LIBTERMINPUT_MACRO;
				} else if (nnums >= 3) {
					/* Parsing for \e[?1000;1015h output. */
					nums[0] -= 32ULL;
				decimal_mouse_tracking_set_press:
					input->mouseevent.event = LIBTERMINPUT_PRESS;
				decimal_mouse_tracking:
					input->mouseevent.type = LIBTERMINPUT_MOUSEEVENT;
					input->mouseevent.x = (size_t)nums[1] + (size_t)!nums[1];
					input->mouseevent.y = (size_t)nums[2] + (size_t)!nums[3];
					input->mouseevent.mods = (enum libterminput_mod)((nums[0] >> 2) & 7ULL);
					if (nums[0] & 32)
						input->mouseevent.event = LIBTERMINPUT_MOTION;
					nums[0] = (nums[0] & 3ULL) | ((nums[0] >> 4) & ~3ULL);
					if (nums[0] < 4) {
						nums[0] = (nums[0] + 1) & 3;
						if (!nums[0] && input->mouseevent.event == LIBTERMINPUT_PRESS)
							input->mouseevent.event = LIBTERMINPUT_RELEASE;
					}
					input->mouseevent.button = (enum libterminput_button)nums[0];
				} else if (!nnums & !(ctx->flags & LIBTERMINPUT_DECSET_1005)) {
					/* Parsing output for legacy mouse tracking output. */
					ctx->mouse_tracking = 0;
					nums = numsbuf;
					nums[0] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
					nums[1] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
					nums[2] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
					nums[0] = (nums[0] - 32ULL) & 255ULL;
					nums[1] = (nums[1] - 32ULL) & 255ULL;
					nums[2] = (nums[2] - 32ULL) & 255ULL;
					if (ctx->stored_head == ctx->stored_tail)
						ctx->stored_head = ctx->stored_tail = 0;
					goto decimal_mouse_tracking_set_press;
				} else if (!nnums) {
					/* Parsing for semi-legacy \e[?1000;1005h output. */
					ctx->mouse_tracking = 0;
					nums = numsbuf;
					pos = ctx->stored_tail;
					nums[0] = utf8_decode(ctx->stored, &ctx->stored_tail);
					if (nums[0] <= 32) {
						ctx->stored_tail = pos;
						goto suppress;
					}
					pos = ctx->stored_tail;
					nums[1] = utf8_decode(ctx->stored, &ctx->stored_tail);
					if (nums[1] <= 32) {
						ctx->stored_tail = pos;
						goto suppress;
					}
					pos = ctx->stored_tail;
					nums[2] = utf8_decode(ctx->stored, &ctx->stored_tail);
					if (nums[2] <= 32) {
						ctx->stored_tail = pos;
						goto suppress;
					}
					nums[0] = nums[0] - 32ULL;
					nums[1] = nums[1] - 32ULL;
					nums[2] = nums[2] - 32ULL;
					if (ctx->stored_head == ctx->stored_tail)
						ctx->stored_head = ctx->stored_tail = 0;
					goto decimal_mouse_tracking_set_press;
				} else {
					goto suppress;
				}
				break;
			case 'P':
				if ((ctx->flags & LIBTERMINPUT_AWAITING_CURSOR_POSITION) && nnums == 2) {
					input->position.type = LIBTERMINPUT_CURSOR_POSITION;
					input->position.y = (size_t)nums[0] + (size_t)!nums[0];
					input->position.x = (size_t)nums[1] + (size_t)!nums[1];
				} else {
					input->keypress.key = LIBTERMINPUT_F1;
					if (ctx->flags & LIBTERMINPUT_PAUSE_ON_CSI_P)
						input->keypress.key = LIBTERMINPUT_PAUSE;
				}
				break;
			case 'Q': input->keypress.key = LIBTERMINPUT_F2;    break;
			case 'R': input->keypress.key = LIBTERMINPUT_F3;    break;
			case 'S': input->keypress.key = LIBTERMINPUT_F4;    break;
			case 'T':
				/* Parsing output for legacy mouse highlight tracking output. (\e[?1001h) */
				ctx->mouse_tracking = 0;
				nums = numsbuf;
				nums[0] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
				nums[1] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
				nums[2] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
				nums[3] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
				nums[4] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
				nums[5] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
				nums[0] = (nums[0] - 32ULL) & 255ULL;
				nums[1] = (nums[1] - 32ULL) & 255ULL;
				nums[2] = (nums[2] - 32ULL) & 255ULL;
				nums[3] = (nums[3] - 32ULL) & 255ULL;
				nums[4] = (nums[4] - 32ULL) & 255ULL;
				nums[5] = (nums[5] - 32ULL) & 255ULL;
				if (ctx->stored_head == ctx->stored_tail)
					ctx->stored_head = ctx->stored_tail = 0;
				input->mouseevent.type = LIBTERMINPUT_MOUSEEVENT;
				input->mouseevent.event = LIBTERMINPUT_HIGHLIGHT_OUTSIDE;
				input->mouseevent.mods = 0;
				input->mouseevent.button = LIBTERMINPUT_BUTTON1;
				input->mouseevent.start_x = (size_t)nums[0];
				input->mouseevent.start_y = (size_t)nums[1];
				input->mouseevent.end_x = (size_t)nums[2];
				input->mouseevent.end_y = (size_t)nums[3];
				input->mouseevent.x = (size_t)nums[4];
				input->mouseevent.y = (size_t)nums[5];
				break;
			case 'U': input->keypress.key = LIBTERMINPUT_NEXT;  break;
			case 'V': input->keypress.key = LIBTERMINPUT_PRIOR; break;
			case 'Z':
				if (!(ctx->flags & LIBTERMINPUT_SEPARATE_BACKTAB)) {
					input->keypress.key = LIBTERMINPUT_TAB;
					input->keypress.mods |= LIBTERMINPUT_SHIFT;
				} else {
					input->keypress.key = LIBTERMINPUT_BACKTAB;
				}
				break;
			case 'a':
				input->keypress.key = LIBTERMINPUT_UP;
				input->keypress.mods |= LIBTERMINPUT_SHIFT;
				break;
			case 'b':
				input->keypress.key = LIBTERMINPUT_DOWN;
				input->keypress.mods |= LIBTERMINPUT_SHIFT;
				break;
			case 'c':
				input->keypress.key = LIBTERMINPUT_RIGHT;
				input->keypress.mods |= LIBTERMINPUT_SHIFT;
				break;
			case 'd':
				input->keypress.key = LIBTERMINPUT_LEFT;
				input->keypress.mods |= LIBTERMINPUT_SHIFT;
				break;
			case 'n':
				if (nnums == 1 && nums[0] == '0') {
					input->type = LIBTERMINPUT_TERMINAL_IS_OK;
				} else if (nnums == 1 && nums[0] == '3') {
					input->type = LIBTERMINPUT_TERMINAL_IS_NOT_OK;
				} else {
					goto suppress;
				}
				break;
			case 't':
				/* Parsing output for legacy mouse highlight tracking output (\e[?1001h). */
				ctx->mouse_tracking = 0;
				nums = numsbuf;
				nums[0] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
				nums[1] = (unsigned long long int)(unsigned char)ctx->stored[ctx->stored_tail++];
				nums[0] = (nums[0] - 32ULL) & 255ULL;
				nums[1] = (nums[1] - 32ULL) & 255ULL;
				if (ctx->stored_head == ctx->stored_tail)
					ctx->stored_head = ctx->stored_tail = 0;
				input->mouseevent.type = LIBTERMINPUT_MOUSEEVENT;
				input->mouseevent.event = LIBTERMINPUT_HIGHLIGHT_INSIDE;
				input->mouseevent.mods = 0;
				input->mouseevent.button = LIBTERMINPUT_BUTTON1;
				input->mouseevent.x = (size_t)nums[0];
				input->mouseevent.y = (size_t)nums[1];
				break;
			case 'u':
				if (nums[0] > 0x10FFFFULL) {
					input->type = LIBTERMINPUT_NONE;
					break;
				}
				encode_utf8(nums[0], input->keypress.symbol);
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
					goto suppress;
				}
				if (25 <= nums[0] && nums[0] <= 34)
					input->keypress.mods |= LIBTERMINPUT_SHIFT;
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
				if (ctx->key[1] == '<' && (ctx->key[2] == 'M' || ctx->key[2] == 'm') && nnums >= 3) {
					/* Parsing for \e[?1003;1006h output. */
					input->mouseevent.event = LIBTERMINPUT_PRESS;
					if (ctx->key[2] == 'm')
						input->mouseevent.event = LIBTERMINPUT_RELEASE;
					goto decimal_mouse_tracking;
				} else {
					goto suppress;
				}
			}
			break;
		default:
			goto suppress;
		}
		break;

	case 'O':
		switch (!ctx->key[2] ? ctx->key[1] : 0) {
		case 'A': input->keypress.key = LIBTERMINPUT_UP;              break;
		case 'B': input->keypress.key = LIBTERMINPUT_DOWN;            break;
		case 'C': input->keypress.key = LIBTERMINPUT_RIGHT;           break;
		case 'D': input->keypress.key = LIBTERMINPUT_LEFT;            break;
		case 'H': input->keypress.key = LIBTERMINPUT_HOME;            break;
		case 'F': input->keypress.key = LIBTERMINPUT_END;             break;
		case 'P': input->keypress.key = LIBTERMINPUT_F1;              break;
		case 'Q': input->keypress.key = LIBTERMINPUT_F2;              break;
		case 'R': input->keypress.key = LIBTERMINPUT_F3;              break;
		case 'S': input->keypress.key = LIBTERMINPUT_F4;              break;
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
		case 'k': input->keypress.key = LIBTERMINPUT_KEYPAD_PLUS;     break;
		case 'm': input->keypress.key = LIBTERMINPUT_KEYPAD_MINUS;    break;
		case 'j': input->keypress.key = LIBTERMINPUT_KEYPAD_TIMES;    break;
		case 'o': input->keypress.key = LIBTERMINPUT_KEYPAD_DIVISION; break;
		case 'n': input->keypress.key = LIBTERMINPUT_KEYPAD_DECIMAL;  break;
		case 'l': input->keypress.key = LIBTERMINPUT_KEYPAD_COMMA;    break;
		case 'b': input->keypress.key = LIBTERMINPUT_KEYPAD_POINT;    break;
		case 'M': input->keypress.key = LIBTERMINPUT_KEYPAD_ENTER;    break;
		default:
			goto suppress;
		}
		break;

	default:
		/* This shouldn't happen (without goto) */
	suppress:
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
	ssize_t rd;

	if (!ctx->inited) {
		ctx->inited = 1;
		memset(input, 0, sizeof(*input));
	} else if (input->type == LIBTERMINPUT_KEYPRESS && input->keypress.times > 1) {
		input->keypress.times -= 1;
		return 1;
	}

	if (ctx->bracketed_paste)
		return read_bracketed_paste(fd, input, ctx);
	if (!ctx->mouse_tracking) {
		r = read_input(fd, &ret, ctx);
		if (r <= 0)
			return r;
	} else if (ctx->mouse_tracking == 1) {
		if (ctx->stored_tail == sizeof(ctx->stored)) {
			memmove(ctx->stored, &ctx->stored[ctx->stored_tail], ctx->stored_head - ctx->stored_tail);
			ctx->stored_tail -= ctx->stored_head;
			ctx->stored_head = 0;
		}
		rd = read(fd, &ctx->stored[ctx->stored_head], 1);
		if (rd <= 0)
			return (int)rd;
		ctx->stored_head += 1;
		p = strchr(ctx->key, '\0');
		goto continue_incomplete;
	} else {
		if (ctx->stored_tail > sizeof(ctx->stored) - (size_t)ctx->mouse_tracking) {
			memmove(ctx->stored, &ctx->stored[ctx->stored_tail], ctx->stored_head - ctx->stored_tail);
			ctx->stored_tail -= ctx->stored_head;
			ctx->stored_head = 0;
		}
		rd = read(fd, &ctx->stored[ctx->stored_head], (size_t)ctx->mouse_tracking - (ctx->stored_head - ctx->stored_tail));
		if (rd <= 0)
			return (int)rd;
		ctx->stored_head += (size_t)rd;
		p = strchr(ctx->key, '\0');
		goto continue_incomplete;
	}

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
	continue_incomplete:
		if (!isalpha(p[-1]) && p[-1] != '~') {
			input->type = LIBTERMINPUT_NONE;
			return 1;
		} else if (ctx->key[0] == '[' && ctx->key[1] == '<' && p == &ctx->key[2]) {
			input->type = LIBTERMINPUT_NONE;
			return 1;
		} else if (ctx->key[0] == '[' && ctx->key[1] == 'M' && (ctx->flags & LIBTERMINPUT_MACRO_ON_CSI_M)) {
			/* complete */
		} else if (ctx->key[0] == '[' && ctx->key[1] == 'M' && (ctx->flags & LIBTERMINPUT_DECSET_1005)) {
			ctx->mouse_tracking = 1;
			if (ctx->stored_head == ctx->stored_tail) {
				input->type = LIBTERMINPUT_NONE;
				return 1;
			}
			n = 0;
			r = check_utf8_char(&ctx->stored[ctx->stored_tail + n], &m, ctx->stored_head - (ctx->stored_tail + n));
			n += m;
			if (!r) {
				input->type = LIBTERMINPUT_NONE;
				return 1;
			} else if (r < 0) {
				ctx->mouse_tracking = 0;
				input->type = LIBTERMINPUT_NONE;
				ctx->stored_tail += n;
				return 1;
			}
			r = check_utf8_char(&ctx->stored[ctx->stored_tail + n], &m, ctx->stored_head - (ctx->stored_tail + n));
			n += m;
			if (!r) {
				input->type = LIBTERMINPUT_NONE;
				return 1;
			} else if (r < 0) {
				ctx->mouse_tracking = 0;
				input->type = LIBTERMINPUT_NONE;
				ctx->stored_tail += n;
				return 1;
			}
			r = check_utf8_char(&ctx->stored[ctx->stored_tail + n], &m, ctx->stored_head - (ctx->stored_tail + n));
			n += m;
			if (!r) {
				input->type = LIBTERMINPUT_NONE;
				return 1;
			} else if (r < 0) {
				ctx->mouse_tracking = 0;
				input->type = LIBTERMINPUT_NONE;
				ctx->stored_tail += n;
				return 1;
			}
		} else if (ctx->key[0] == '[' && ctx->key[1] == 'M' && ctx->stored_head - ctx->stored_tail < 3) {
			ctx->mouse_tracking = 3;
			input->type = LIBTERMINPUT_NONE;
			return 1;
		} else if (ctx->key[0] == '[' && ctx->key[1] == 't' && ctx->stored_head - ctx->stored_tail < 2) {
			ctx->mouse_tracking = 2;
			input->type = LIBTERMINPUT_NONE;
			return 1;
		} else if (ctx->key[0] == '[' && ctx->key[1] == 'T' && ctx->stored_head - ctx->stored_tail < 6) {
			ctx->mouse_tracking = 6;
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
		input->type = LIBTERMINPUT_NONE;
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
		case 033:
			input->keypress.key = LIBTERMINPUT_ESC;
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


int
libterminput_set_flags(struct libterminput_state *ctx, enum libterminput_flags flags)
{
	ctx->flags |= flags;
	return 0;
}


int
libterminput_clear_flags(struct libterminput_state *ctx, enum libterminput_flags flags)
{
	ctx->flags |= flags;
	ctx->flags ^= flags;
	return 0;
}


extern inline int libterminput_is_ready(union libterminput_input *input, struct libterminput_state *ctx);
