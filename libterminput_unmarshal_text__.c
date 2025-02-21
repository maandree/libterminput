/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_unmarshal_text__(struct libterminput_unmarshaller *how, struct libterminput_text *what)
{
	what->type = LIBTERMINPUT_TEXT;
	if (how->load(how, &what->nbytes, sizeof(what->nbytes)))
		return -1;
	if (what->nbytes > sizeof(what->bytes)) {
		errno = EINVAL;
		return -1;
	}
	return how->load(how, what->bytes, what->nbytes);
}
