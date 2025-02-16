/* See LICENSE file for copyright and license details. */
#include "common.h"


unsigned long long int
libterminput_utf8_decode__(const char *s, size_t *ip)
{
	unsigned long long int cp = 0;
	size_t len;

	/* Parse the first byte, to get the highest codepoint bits and the encoding length */
	if ((s[*ip] & 0x80) == 0) {
		return (unsigned long long int)s[(*ip)++];
	} else if ((s[*ip] & 0xE0) == 0xC0) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xC0U);
		len = 2U;
		goto need_1;
	} else if ((s[*ip] & 0xF0) == 0xE0) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xE0U);
		len = 3U;
		goto need_2;
	} else if ((s[*ip] & 0xF8) == 0xF0) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xF0U);
		len = 4U;
		goto need_3;
	} else if ((s[*ip] & 0xFC) == 0xF8) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xF8U);
		len = 5U;
		goto need_4;
	} else if ((s[*ip] & 0xFE) == 0xFC) {
		cp = (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0xFCU);
		len = 6U;
		goto need_5;
	}

	/* Parse continuation bytes; check marked as continuation the get codepoint bits */
need_5:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

need_4:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

need_3:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

need_2:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

need_1:
	if ((s[*ip] & 0xC0) != 0x80) return 0;
	cp <<= 6;
	cp |= (unsigned long long int)((unsigned char)s[(*ip)++] ^ 0x80U);

	/* Check that encoded codepoint is encoded with the minimum possible length */
	if (cp < 1ULL << (7 + 0 * 6))
		return 0;
	if (cp < 1ULL << (5 + 1 * 6))
		return len > 2U ? 0ULL : cp;
	if (cp < 1ULL << (4 + 2 * 6))
		return len > 3U ? 0ULL : cp;
	if (cp < 1ULL << (3 + 3 * 6))
		return len > 4U ? 0ULL : cp;
	if (cp < 1ULL << (2 + 4 * 6))
		return len > 5U ? 0ULL : cp;
	if (cp < 1ULL << (1 + 5 * 6))
		return len > 6U ? 0ULL : cp;

	/* (Let's ignore the 0x10FFFF upper bound.) */

	return 0;
}
