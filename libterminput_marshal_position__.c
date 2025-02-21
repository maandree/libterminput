/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_marshal_position__(struct libterminput_marshaller *how, const struct libterminput_position *what)
{
	return how->store(how, &what->x, sizeof(size_t) * 2U);
}
