/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libterminput_check_utf8_char__(const char *s, size_t size, size_t *len_out)
{
	size_t i;
	*len_out = 0;
	if (!size) {
		return 0;
	} else if ((*s & 0x80) == 0) {
		*len_out = 1U;
		return 1;
	} else if ((*s & 0xE0) == 0xC0) {
		*len_out = 2U;
	} else if ((*s & 0xF0) == 0xE0) {
		*len_out = 3U;
	} else if ((*s & 0xF8) == 0xF0) {
		*len_out = 4U;
	} else if ((*s & 0xFC) == 0xF8) {
		*len_out = 5U;
	} else if ((*s & 0xFE) == 0xFC) {
		*len_out = 6U;
	} else {
		*len_out = 0U;
		return -1;
	}
	for (i = 1; i < *len_out; i++) {
		if (i == size)
			return 0;
		if ((s[i] & 0xC0) != 0x80)
			return -1;
	}
	return 1;
}
