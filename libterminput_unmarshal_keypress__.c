/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_unmarshal_keypress__(struct libterminput_unmarshaller *how, struct libterminput_keypress *what)
{
	what->type = LIBTERMINPUT_KEYPRESS;
	if (how->load(how, &what->key, sizeof(what->key)) ||
	    how->load(how, &what->times, sizeof(what->times)) ||
	    how->load(how, &what->mods, sizeof(what->mods)))
		return -1;
	if ((uintmax_t)what->key > (uintmax_t)LIBTERMINPUT_LAST_KEY__) {
		errno = EINVAL;
		return -1;
	}
	if (what->key == LIBTERMINPUT_SYMBOL) {
		return how->load(how, what->symbol, sizeof(what->symbol));
	} else {
		memset(what->symbol, 0, sizeof(what->symbol));
		return 0;
	}
}
