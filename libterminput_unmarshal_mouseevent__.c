/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_unmarshal_mouseevent__(struct libterminput_unmarshaller *how, struct libterminput_mouseevent *what)
{
	what->type = LIBTERMINPUT_MOUSEEVENT;
	if (how->load(how, &what->event, sizeof(what->event)) ||
	    how->load(how, &what->x, sizeof(size_t) * 2U))
		return -1;
	what->mods = 0;
	what->button = LIBTERMINPUT_BUTTON1;
	if (what->event == LIBTERMINPUT_HIGHLIGHT_OUTSIDE) {
		if (how->load(how, &what->start_x, sizeof(size_t) * 4U))
			return -1;
	} else if (what->event != LIBTERMINPUT_HIGHLIGHT_INSIDE) {
		if (how->load(how, &what->mods, sizeof(what->mods)) ||
		    how->load(how, &what->button, sizeof(what->button)))
			return -1;
	}
	return 0;
}
