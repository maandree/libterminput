/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_read_symbol__(int fd, struct input *input, struct libterminput_state *ctx)
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
			ctx->stored_tail = 1U;
			ctx->stored_head = (size_t)r;
		}
	}

	/* Check if symbol is complete or can be completed, and split out modifier */
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

	} else if ((c & 0xC0) == 0xC0 && c != 0xFF) {
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
		goto again;

	} else {
		/* Single-byte-character */
		input->symbol[0] = (char)c;
		input->symbol[1] = '\0';
		input->mods = ctx->mods;
		ctx->mods = 0;
		return 1;
	}

	/* Incomplete symbol */
	input->symbol[0] = '\0';
	input->mods = -1;
	return 1;
}
