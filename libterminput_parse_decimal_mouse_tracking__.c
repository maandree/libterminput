/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libterminput_parse_decimal_mouse_tracking__(union libterminput_input *input, unsigned long long int nums[3])
{
	input->mouseevent.type = LIBTERMINPUT_MOUSEEVENT;
	input->mouseevent.x = (size_t)nums[1] + (size_t)!nums[1];
	input->mouseevent.y = (size_t)nums[2] + (size_t)!nums[2];
	input->mouseevent.mods = (enum libterminput_mod)((nums[0] >> 2) & 7ULL);
	if (nums[0] & 32)
		input->mouseevent.event = LIBTERMINPUT_MOTION;
	nums[0] = (nums[0] & 3ULL) | ((nums[0] >> 4) & ~3ULL);
	if (nums[0] < 4) {
		nums[0] = (nums[0] + 1) & 3;
		if (!nums[0] && input->mouseevent.event == LIBTERMINPUT_PRESS) {
			input->mouseevent.event = LIBTERMINPUT_RELEASE;
			nums[0] = 1;
		}
	}
	input->mouseevent.button = (enum libterminput_button)nums[0];
}
