/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_unmarshal_state(struct libterminput_unmarshaller *how, struct libterminput_state *what) /* TODO test */
{
	return how->load(how, what, sizeof(*what));
}
