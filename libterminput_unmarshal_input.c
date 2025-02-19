/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_unmarshal_input(struct libterminput_unmarshaller *how, union libterminput_input *what) /* TODO test */
{
	enum libterminput_type type;
	int r;
	if (how->load(how, &what->type, sizeof(what->type)))
		return -1;
	type = what->type;
	if (what->type == LIBTERMINPUT_NONE) {
		what->keypress.key = LIBTERMINPUT_SYMBOL;
		if (how->load(how, &what->type, sizeof(what->type)))
			return -1;
	}
	if (what->type == LIBTERMINPUT_KEYPRESS)
		r = libterminput_unmarshal_keypress__(how, &what->keypress);
	else if (what->type == LIBTERMINPUT_TEXT)
		r = libterminput_unmarshal_text__(how, &what->text);
	else if (what->type == LIBTERMINPUT_MOUSEEVENT)
		r = libterminput_unmarshal_mouseevent__(how, &what->mouseevent);
	else if (what->type == LIBTERMINPUT_CURSOR_POSITION)
		r = libterminput_unmarshal_position__(how, &what->position);
	else
		r = 0;
	what->type = type;
	return r;
}
