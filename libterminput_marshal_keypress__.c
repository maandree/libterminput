/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_marshal_keypress__(struct libterminput_marshaller *how, const struct libterminput_keypress *what) /* TODO test */
{
	if (how->store(how, &what->key, sizeof(what->key)) ||
	    how->store(how, &what->times, sizeof(what->times)) ||
	    how->store(how, &what->mods, sizeof(what->mods)))
		return -1;
	if (what->key == LIBTERMINPUT_SYMBOL)
		return how->store(how, what->symbol, sizeof(what->symbol));
	else
		return 0;
}
