/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_marshal_state(struct libterminput_marshaller *how, const struct libterminput_state *what) /* TODO test */
{
	return how->store(how, what, sizeof(*what));
}
