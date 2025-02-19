/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_unmarshal_position__(struct libterminput_unmarshaller *how, struct libterminput_position *what)
{
	what->type = LIBTERMINPUT_CURSOR_POSITION;
	return how->load(how, &what->x, sizeof(size_t) * 2U);
}
