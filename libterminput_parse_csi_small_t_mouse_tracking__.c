/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libterminput_parse_csi_small_t_mouse_tracking__(union libterminput_input *input, struct libterminput_state *ctx)
{
	unsigned long long int nums[2];

	/* Parsing output for legacy mouse highlight tracking output (\e[?1001h) */
	ctx->mouse_tracking = 0;
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
	input->mouseevent.x = (size_t)nums[0] + (size_t)!nums[0];
	input->mouseevent.y = (size_t)nums[1] + (size_t)!nums[1];
}
