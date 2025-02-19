/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_marshal_mouseevent__(struct libterminput_marshaller *how, const struct libterminput_mouseevent *what)
{
	if (how->store(how, &what->event, sizeof(what->event)) ||
	    how->store(how, &what->x, sizeof(size_t) * 2U))
		return -1;
	if (what->event == LIBTERMINPUT_HIGHLIGHT_OUTSIDE) {
		if (how->store(how, &what->start_x, sizeof(size_t) * 4U))
			return -1;
	} else if (what->event != LIBTERMINPUT_HIGHLIGHT_INSIDE) {
		if (how->store(how, &what->mods, sizeof(what->mods)) ||
		    how->store(how, &what->button, sizeof(what->button)))
			return -1;
	}
	return 0;
}
