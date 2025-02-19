/* See LICENSE file for copyright and license details. */
#include "libterminput.h"

#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>


#if defined(__GNUC__)
# define HIDDEN __attribute__((__visibility__("hidden")))
#else
# define HIDDEN
#endif


/**
 * Singlar read symbol
 */
struct input {
	/**
	 * Applied modifier keys
	 */
	enum libterminput_mod mods;

	/**
	 * The read symbol; NUL-byte terminated
	 */
	char symbol[7];
};


/**
 * Encode a Unicode codepoint in UTF-8
 * 
 * @param  codepoint  The codepoint to encode
 * @param  buffer     Output buffer for the NUL-byte terminated UTF-8 encoding of `codepoint`
 */
HIDDEN void libterminput_encode_utf8__(unsigned long long int codepoint, char buffer[7]);

/**
 * Validate an UTF-8 byte sequence, up to one codepoint encoding
 *
 * @param   s        The buffer to read from
 * @param   size     The number of bytes available in `s`
 * @param   len_out  Output parameter for the encoding length of the
 *                   codepoint encoded at the beginning of `s`
 * @return           1 if `s` begins with a valid codepoint,
 *                   0 if `size` is too small to determine the validity,
 *                   -1 if the byte sequence is illegal
 */
HIDDEN int libterminput_check_utf8_char__(const char *s, size_t size, size_t *len_out);

/**
 * Decode a Unicode codepoint encoded in UTF-8
 * 
 * @param   s   The buffer to read from
 * @param   ip  Pointer to the current position in `s`, will be updated
 * @return      The first encode codepoint, 0 if invalid (or if 0)
 */
HIDDEN unsigned long long int libterminput_utf8_decode__(const char *s, size_t *ip);

/**
 * Get input, from the terminal that, that appear after
 * the start marker for a bracketed paste
 * 
 * @param   fd     The file descriptor to the terminal
 * @param   input  Output parameter for input
 * @param   ctx    State for the terminal, parts of the state may be stored in `input`
 * @return         1 normally, 0 on end of input, -1 on error
 *
 * @throws  Any reason specified for read(3)
 */
HIDDEN int libterminput_read_bracketed_paste__(int fd, union libterminput_input *input, struct libterminput_state *ctx);

/**
 * Parse mouse tracking event data
 * 
 * @param  input  Output parameter for the parsed event
 * @param  nums   The numbers assoicated with the event
 */
HIDDEN void libterminput_parse_decimal_mouse_tracking__(union libterminput_input *input, unsigned long long int nums[3]);

/**
 * Parse a CSI M mouse tracking event
 * 
 * @param  input  Output parameter for the parsed event
 * @param  ctx    State for the terminal, parts of the state may be stored in `input`
 * @param  nums   Numbers insert reported for the event (between CSI and M)
 * @param  nnums  Number of elements in `nums`
 */
HIDDEN void libterminput_parse_csi_m_mouse_tracking__(union libterminput_input *input, struct libterminput_state *ctx,
                                                      unsigned long long int *nums, size_t nnums);

/**
 * Parse a CSI T mouse tracking event
 *
 * @param  input  Output parameter for the parsed event
 * @param  ctx    State for the terminal, parts of the state may be stored in `input`
 */
HIDDEN void libterminput_parse_csi_t_mouse_tracking__(union libterminput_input *input, struct libterminput_state *ctx);

/**
 * Parse a CSI t mouse tracking event
 *
 * @param  input  Output parameter for the parsed event
 * @param  ctx    State for the terminal, parts of the state may be stored in `input`
 */
HIDDEN void libterminput_parse_csi_small_t_mouse_tracking__(union libterminput_input *input, struct libterminput_state *ctx);

/**
 * Parse a complete, atomic input sequence out side of a bracketed paste
 *
 * @param  input  Output parameter for the parsed event
 * @param  ctx    State for the terminal, parts of the state may be stored in `input`
 */
HIDDEN void libterminput_parse_sequence__(union libterminput_input *input, struct libterminput_state *ctx);

/**
 * Read a singular symbol from the terminal
 * 
 * @param   fd     The file descriptor to the terminal
 * @param   input  Output parameter for input
 * @param   ctx    State for the terminal, parts of the state may be stored in `input`
 * @return         1 normally, 0 on end of input, -1 on error
 * 
 * @throws  Any reason specified for read(3)
 */
HIDDEN int libterminput_read_symbol__(int fd, struct input *input, struct libterminput_state *ctx);


#undef HIDDEN
