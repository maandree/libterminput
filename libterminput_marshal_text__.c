/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_marshal_text__(struct libterminput_marshaller *how, const struct libterminput_text *what)
{
	if (what->nbytes > sizeof(what->bytes))
		abort();
	if (how->store(how, &what->nbytes, sizeof(what->nbytes)) ||
	    how->store(how, what->bytes, what->nbytes))
		return -1;
	return 0;
}
