/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_read_bracketed_paste__(int fd, union libterminput_input *input, struct libterminput_state *ctx)
{
	ssize_t r;
	size_t n;
	size_t i;

	/* Unfortunately there is no standard for how to handle pasted ESC's,
	 * not even ESC [201~ or ESC ESC. Terminates seem to just paste ESC as
	 * is, so we cannot do anything about them, however, a good terminal
	 * would stop the paste at the ~ in ESC [201~, send ~ as normal, and
	 * then continue the brackated paste mode. */

	/* Check for bracketed paste end marker to output LIBTERMINPUT_BRACKETED_PASTE_END
	 * and stop, and read more if we don't have it; the marker will be at the
	 * beginning as the function will stop when it encounteres it and output the
	 * text pasted before it */
	if (ctx->stored_head - ctx->stored_tail) {
		/* If we have input buffered, unpause and handle it */
		ctx->paused = 0;
		n = ctx->stored_head - ctx->stored_tail;
		if (!strncmp(&ctx->stored[ctx->stored_tail], "\033[201~", n < 6U ? n : 6U)) {
			/* If starting with bracketed paste end marker, output LIBTERMINPUT_BRACKETED_PASTE_END, */
			if (n >= 6U) {
				ctx->stored_tail += 6U;
				if (ctx->stored_tail == ctx->stored_head)
					ctx->stored_tail = ctx->stored_head = 0;
				ctx->bracketed_paste = 0;
				input->type = LIBTERMINPUT_BRACKETED_PASTE_END;
				return 1;
			}
			/* otherwise, but if the buffered input is a truncating of the marker,
			 * move over the data from the stored input buffer to the input buffer
			 * and store continue reading input */
			input->text.nbytes = ctx->stored_head - ctx->stored_tail;
			memcpy(input->text.bytes, &ctx->stored[ctx->stored_tail], input->text.nbytes);
			r = read(fd, &input->text.bytes[input->text.nbytes], sizeof(input->text.bytes) - input->text.nbytes);
			if (r <= 0)
				return (int)r;
			input->text.nbytes += (size_t)r;
			ctx->stored_head = ctx->stored_tail = 0;
		} else {
			/* If the buffered input does not begin with the bracketed paste end marker,
			 * or a truncation of it, move over the data from the stored input buffer
			 * to the input buffer */
			input->text.nbytes = ctx->stored_head - ctx->stored_tail;
			memcpy(input->text.bytes, &ctx->stored[ctx->stored_tail], input->text.nbytes);
			ctx->stored_head = ctx->stored_tail = 0;
		}
	} else {
		/* If we don't have any input buffered, read some */
		r = read(fd, input->text.bytes, sizeof(input->text.bytes));
		if (r <= 0)
			return (int)r;
		input->text.nbytes = (size_t)r;
	}

	/* Count the number of bytes available before a bracketed paste end
	 * marker, or a truncation of it at the end of the input buffer */
	for (n = 0; n + 5U < input->text.nbytes; n++) {
		if (!strncmp(&input->text.bytes[n], "\033[201~", 6U))
			break;
	}
	for (i = 5U; i--;) {
		if (n + i < input->text.nbytes) {
			if (!strncmp(&input->text.bytes[n], "\033[201~", i + 1U))
				break;
			n += 1;
		}
	}

	/* Of there was pasted input, output it */
	if (n) {
		ctx->stored_tail = 0;
		ctx->stored_head = input->text.nbytes - n;
		memcpy(ctx->stored, &input->text.bytes[n], ctx->stored_head);
		input->text.nbytes = n;
		input->text.type = LIBTERMINPUT_TEXT;
		return 1;
	}

	/* If the input is solely a truncation of the bracketed paste
	 * end marker, output that we do not have any complete input */
	if (input->text.nbytes < 6U) {
		input->text.type = LIBTERMINPUT_NONE;
		memcpy(ctx->stored, input->text.bytes, input->text.nbytes);
		ctx->stored_tail = 0;
		ctx->stored_head = input->text.nbytes;
		ctx->paused = 1;
		return 1;
	}

	/* If the input starts with a bracketed paste end marker,
	 * output it and store the rest of the input buffer for
	 * later processing */
	ctx->stored_tail = 0;
	ctx->stored_head = input->text.nbytes - 6U;
	memcpy(ctx->stored, &input->text.bytes[6], ctx->stored_head);
	if (ctx->stored_tail == ctx->stored_head)
		ctx->stored_tail = ctx->stored_head = 0;
	ctx->bracketed_paste = 0;
	input->type = LIBTERMINPUT_BRACKETED_PASTE_END;
	return 1;
}
