/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libterminput_parse_csi_m_mouse_tracking__(union libterminput_input *input, struct libterminput_state *ctx,
                                          unsigned long long int *nums, size_t nnums)
{
	unsigned long long int numsbuf[3];
	size_t pos;

	if (nnums >= 3U) {
		/* Parsing for \e[?1000;1015h output */
		nums[0] -= 32ULL;

	} else if (!nnums && (ctx->flags & LIBTERMINPUT_DECSET_1005)) {
		/* Parsing for semi-legacy \e[?1000;1005h output */
		ctx->mouse_tracking = 0;
		nums = numsbuf;
		pos = ctx->stored_tail;
		if ((nums[0] = libterminput_utf8_decode__(ctx->stored, &ctx->stored_tail)) < 32 ||
		    (nums[1] = libterminput_utf8_decode__(ctx->stored, &ctx->stored_tail)) < 32 ||
		    (nums[2] = libterminput_utf8_decode__(ctx->stored, &ctx->stored_tail)) < 32) {
			ctx->stored_tail = pos;
			input->keypress.key = LIBTERMINPUT_MACRO;
			return;
		}
		nums[0] = nums[0] - 32ULL;
		nums[1] = nums[1] - 32ULL;
		nums[2] = nums[2] - 32ULL;
		if (ctx->stored_head == ctx->stored_tail)
			ctx->stored_head = ctx->stored_tail = 0;

	} else if (!nnums) {
		/* Parsing output for legacy mouse tracking output */
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

	} else {
		NOTHING(input);
		return;
	}

	input->mouseevent.event = LIBTERMINPUT_PRESS;
	libterminput_parse_decimal_mouse_tracking__(input, nums);
}
