/* See LICENSE file for copyright and license details. */
#include "common.h"


#if defined(__GNUC__)
__attribute__((__const__))
#endif
int
libterminput_init(struct libterminput_state *ctx, int fd)
{
	(void) ctx;
	(void) fd;
	return -1;
}
