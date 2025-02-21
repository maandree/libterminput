/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_unmarshal_input(struct libterminput_unmarshaller *how, union libterminput_input *what)
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
	switch ((int)what->type) {
	case LIBTERMINPUT_KEYPRESS:
		r = libterminput_unmarshal_keypress__(how, &what->keypress);
		break;
	case LIBTERMINPUT_TEXT:
		r = libterminput_unmarshal_text__(how, &what->text);
		break;
	case LIBTERMINPUT_MOUSEEVENT:
		r = libterminput_unmarshal_mouseevent__(how, &what->mouseevent);
		break;
	case LIBTERMINPUT_CURSOR_POSITION:
		r = libterminput_unmarshal_position__(how, &what->position);
		break;
	case LIBTERMINPUT_NONE:
	case LIBTERMINPUT_BRACKETED_PASTE_START:
	case LIBTERMINPUT_BRACKETED_PASTE_END:
	case LIBTERMINPUT_TERMINAL_IS_OK:
	case LIBTERMINPUT_TERMINAL_IS_NOT_OK:
		r = 0;
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	what->type = type;
	return r;
}
