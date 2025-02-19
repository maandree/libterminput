/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_marshal_input(struct libterminput_marshaller *how, const union libterminput_input *what) /* TODO test */
{
	enum libterminput_type type = what->type;
	if (how->store(how, &type, sizeof(type)))
		return -1;
	if (type == LIBTERMINPUT_NONE) {
		if (what->keypress.key == LIBTERMINPUT_SYMBOL)
			type = LIBTERMINPUT_KEYPRESS;
		if (how->store(how, &type, sizeof(type)))
			return -1;
	}
	if (type == LIBTERMINPUT_KEYPRESS)
		return libterminput_marshal_keypress__(how, &what->keypress);
	else if (type == LIBTERMINPUT_TEXT)
		return libterminput_marshal_text__(how, &what->text);
	else if (type == LIBTERMINPUT_MOUSEEVENT)
		return libterminput_marshal_mouseevent__(how, &what->mouseevent);
	else if (type == LIBTERMINPUT_CURSOR_POSITION)
		return libterminput_marshal_position__(how, &what->position);
	else
		return 0;
}
