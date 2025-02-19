/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_read(int fd, union libterminput_input *input, struct libterminput_state *ctx)
{
	struct input ret;
	size_t n, m;
	char *p;
	int r, i;
	ssize_t rd;

	if (!ctx->inited) {
		/* Initialise structures */
		ctx->inited = 1;
		memset(input, 0, sizeof(*input));
		/* The user should have set all bytes in `*ctx` to 0, and it doesn't
		 * contain any pointers, and in fact all initial values should be 0,
		 * so we do not need to modify `*ctx` except mark it as initialised */
	} else if (input->type == LIBTERMINPUT_KEYPRESS && input->keypress.times > 1) {
		/* For repeated input, unless counter is reset by user, report
		 * the same event again (and decrease the counter) */
		input->keypress.times -= 1;
		return 1;
	}

	/* If in a bracketed paste, use the reading function for that mode */
	if (ctx->bracketed_paste)
		return libterminput_read_bracketed_paste__(fd, input, ctx);

	/* If we are on an incomplete mouse tracking event, read more raw input,
	 * otherwise read one symbol */
	if (!ctx->mouse_tracking) {
		r = libterminput_read_symbol__(fd, &ret, ctx);
		if (r <= 0) {
			if (!r || ctx->blocked || !ctx->queued || errno != EAGAIN)
				return r;
			ctx->blocked = 1;
			ctx->queued = 0;
			input->type = LIBTERMINPUT_KEYPRESS;
			ctx->mods = 0;
			ctx->meta = 0;
			return 1;
		}
		ctx->blocked = 0;
		ctx->queued = 0;
	} else {
		if (ctx->stored_tail > sizeof(ctx->stored) - (size_t)ctx->mouse_tracking) {
			memmove(ctx->stored, &ctx->stored[ctx->stored_tail], ctx->stored_head - ctx->stored_tail);
			ctx->stored_tail -= ctx->stored_head;
			ctx->stored_head = 0;
		}
		rd = read(fd, &ctx->stored[ctx->stored_head], sizeof(ctx->stored) - ctx->stored_head);
		if (rd <= 0)
			return (int)rd;
		ctx->stored_head += (size_t)rd;
		p = strchr(ctx->key, '\0');
		goto continue_incomplete;
	}

again:
	if (!*ret.symbol) {
		/* Incomplete input, specifically only Meta/ESC's */
		if (ctx->meta < 3) {
			/* Up to two Meta/ESC, wait until a third or something else is read,
			 * or if ESC on block, report as ESC or Meta+ESC */
			if (ctx->flags & LIBTERMINPUT_ESC_ON_BLOCK) {
				input->keypress.key = LIBTERMINPUT_ESC;
				input->keypress.times = 1;
				input->keypress.mods = ctx->mods;
				input->keypress.symbol[0] = '\0';
				if (ctx->meta > 1)
					input->keypress.mods |= LIBTERMINPUT_META;
				ctx->queued = 1;
				goto none;
			}
			goto none;
		}
		/* Three ESC's */
		input->type = LIBTERMINPUT_KEYPRESS;
		input->keypress.key = LIBTERMINPUT_ESC;
		input->keypress.times = 3;
		input->keypress.mods = 0;
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
			goto none;
		}
		p = stpcpy(&ctx->key[n], ret.symbol);
		/* Check if sequence is complete */
	continue_incomplete:
		if (!isalpha(p[-1]) && p[-1] != '~' && p[-1] != '@' && p[-1] != '^' && p[-1] != '$') {
			goto none;
		} else if (ctx->key[0] == '[' && ctx->key[1] == '<' && p == &ctx->key[2]) {
			goto none;
		} else if (ctx->key[0] == '[' && ctx->key[1] == 'M' && (ctx->flags & LIBTERMINPUT_MACRO_ON_CSI_M)) {
			/* complete */
			/* TODO optionally this should also be the case if blocked */
		} else if (ctx->key[0] == '[' && ctx->key[1] == 'M' && (ctx->flags & LIBTERMINPUT_DECSET_1005)) {
			ctx->mouse_tracking = 1;
			n = ctx->stored_tail;
			for (i = 0; i < 3; i++) {
				r = libterminput_check_utf8_char__(&ctx->stored[n], ctx->stored_head - n, &m);
				if (r > 0) {
					n += m;
					continue;
				}
				if (!r)
					goto none;
				ctx->mouse_tracking = 0;
				input->type = LIBTERMINPUT_KEYPRESS;
				input->keypress.key = LIBTERMINPUT_MACRO;
				input->keypress.mods = ret.mods;
				input->keypress.times = 1;
				if (ctx->meta > 1)
					input->keypress.mods |= LIBTERMINPUT_META;
				ctx->meta = 0;
				ctx->key[0] = '\0';
				return 1;
			}
		} else if (ctx->key[0] == '[' && ctx->key[1] == 'M' && ctx->stored_head - ctx->stored_tail < 3U) {
			ctx->mouse_tracking = 3;
			goto none;
		} else if (ctx->key[0] == '[' && ctx->key[1] == 't' && ctx->stored_head - ctx->stored_tail < 2U) {
			ctx->mouse_tracking = 2;
			goto none;
		} else if (ctx->key[0] == '[' && ctx->key[1] == 'T' && ctx->stored_head - ctx->stored_tail < 6U) {
			ctx->mouse_tracking = 6;
			goto none;
		}
		/* Parse the complete sequence */
		libterminput_parse_sequence__(input, ctx);
		/* Reset */
		ctx->meta = 0;
		ctx->key[0] = '\0';

	} else if (ctx->meta && (!strcmp(ret.symbol, "[") || !strcmp(ret.symbol, "O") || !strcmp(ret.symbol, "?"))) {
		/* ESC [, ESC O, or ESC ? is used as the beginning of most special keys */
		stpcpy(ctx->key, ret.symbol);
		goto none;

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
		input->keypress.symbol[0] = '\0';
		switch (ret.symbol[1] ? 0 : ret.symbol[0]) {
		case 127:
		case '\b': input->keypress.key = LIBTERMINPUT_ERASE; break;
		case '\t': input->keypress.key = LIBTERMINPUT_TAB;   break;
		case '\n': input->keypress.key = LIBTERMINPUT_ENTER; break;
		case 033:  input->keypress.key = LIBTERMINPUT_ESC;   break;
		default:
			input->keypress.key = LIBTERMINPUT_SYMBOL;
			stpcpy(input->keypress.symbol, ret.symbol);
			break;
		}
	}

	return 1;

none:
	input->type = LIBTERMINPUT_NONE;
	return 1;
}
