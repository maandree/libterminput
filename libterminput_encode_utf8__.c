/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libterminput_encode_utf8__(unsigned long long int codepoint, char buffer[7])
{
	static const char masks[6] = {
		(char)0x00, /* 1 byte = 0 high set bits, */
		(char)0xC0, /* 2 bytes = 2 high set bits, */
		(char)0xE0, /* 3 bytes = 3 high set bits, ... */
		(char)0xF0,
		(char)0xF8,
		(char)0xFC  /* 6 bytes = 3 high set bits */
	};
	static const unsigned long long int limits[6] = {
		1ULL << (7 + 0 * 6), /* 1 byte has room for 7 codepoint encoding bits, */
		1ULL << (5 + 1 * 6), /* 2 bytes has room for 5 bits in the first by and 6 bits the rest, */
		1ULL << (4 + 2 * 6), /* 3 bytes has room for 4 bits in the first by and 6 bits the rest, ... */
		1ULL << (3 + 3 * 6),
		1ULL << (2 + 4 * 6),
		1ULL << (1 + 5 * 6)  /* 6 bytes has room for 1 bits in the first by and 6 bits the rest */
	};

	size_t len;

	/* Get encoding length for codepoint */
	for (len = 0; codepoint >= limits[len]; len++);

	/* Set the `len` (but 0 if 1) high bits in the first byte
	 * to encode the encoding length of the codepoint */
	buffer[0] = masks[len];

	/* NUL terminate the encoding buffer,
	 * to mark the encode of the encoding */
	buffer[++len] = '\0';

	/* Encode the bites representing the code point
	 * and the length continuation marker bits in
	 * the non-first bytes */
	for (; --len; codepoint >>= 6)
		buffer[len] = (char)((codepoint & 0x3FULL) | 0x80ULL);
	buffer[0] |= (char)codepoint;
}
