/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_unmarshal_state(struct libterminput_unmarshaller *how, struct libterminput_state *what) /* TODO test */
{
	if (how->load(how, what, sizeof(*what)))
		return -1;
	if (what->inited < 0 || what->inited > 1 ||
	    what->bracketed_paste > 1U ||
	    what->meta > 2U ||
	    what->n >= sizeof(what->partial) ||
	    what->npartial >= sizeof(what->partial) ||
	    what->stored_tail > what->stored_head ||
	    what->stored_head > sizeof(what->stored) ||
	    what->unused_bits)
		goto einval;
	switch (what->mouse_tracking) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 6:
		break;
	default:
		goto einval;
	}
	return 0;

einval:
	errno = EINVAL;
	return -1;
}
