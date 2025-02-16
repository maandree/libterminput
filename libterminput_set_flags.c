/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_set_flags(struct libterminput_state *ctx, enum libterminput_flags flags)
{
	ctx->flags |= flags;
	return 0;
}
