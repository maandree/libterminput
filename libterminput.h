/* See LICENSE file for copyright and license details. */
#ifndef LIBTERMINPUT_H
#define LIBTERMINPUT_H

#include <stddef.h>


/**
 * Flags for supporting incompatible input; the user must
 * set or clear his flag after setting or clearing it on
 * the terminal, and the user must make sure that the
 * terminal support this flag if set
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
enum libterminput_flags {
	/**
	 * The sequence CSI M shall be parsed be parse as a DECSET 1005
	 * sequence which is incompatible with legacy mouse tracking
	 *
	 * This flag shall only be set if DECSET 1005 has sent to the
	 * terminal and the user is sure it is supported by the terminal
	 */
	LIBTERMINPUT_DECSET_1005              = 0x0001,

	/**
	 * Parse CSI M as Macro key presses rather than mouse
	 * tracking events
	 * 
	 * This is incompatible with all mouse tracking modes except
	 * DECSET 1006
	 */
	LIBTERMINPUT_MACRO_ON_CSI_M           = 0x0002,

	/**
	 * Parse CSI P as Pause key presses rather than F1 key presses
	 */
	LIBTERMINPUT_PAUSE_ON_CSI_P           = 0x0004,

	/**
	 * Parse CSI @ as Insert key presses rather than a number of
	 * possible special keys combined with the control and shift
	 * modifiers
	 */
	LIBTERMINPUT_INS_ON_CSI_AT            = 0x0008,

	/**
	 * Backtab shall be treated as a separate key, and not be
	 * reported as tab with the shift modifier. This flag is just
	 * a usability issue. Keyboards put backtab on shift+tab,
	 * which is why the tab keycap has both a backward arrow
	 * (backtab) and a forward arrow (tab); but most users are
	 * unfamiliar with backtab, and just see it as shift+tab.
	 */
	LIBTERMINPUT_SEPARATE_BACKTAB         = 0x0010,

	/**
	 * If an ESC is received without anything after it,
	 * return ESC keypress. This is not always desirable
	 * behaviour as the user may manually press ESC to
	 * simulate a keypress that terminal does not support
	 * (yes, this is a real world issue).
	 */
	LIBTERMINPUT_ESC_ON_BLOCK             = 0x0020,

	/**
	 * This flag should be set, by the application, once
	 * the application sends an escape sequence requesting
	 * the terminal to report the cursor's position, and
	 * cleared once the position has been sent by the
	 * terminal and retreived by application
	 * 
	 * This is required for distinguishing cursor position
	 * reports from F3 key presses
	 */
	LIBTERMINPUT_AWAITING_CURSOR_POSITION = 0x0040,

	/**
	 * If CSI M is received without anything after it,
	 * return Macro keypress. Since the user probably
	 * does not have the Macro key, it seems having this
	 * as the default behaviour introduces an unncessary
	 * risk of misparsing input. However, if mouse tracking
	 * is not activated, it makes since to enable this
	 * flag.
	 * 
	 * @since  1.1
	 */
	LIBTERMINPUT_MACRO_ON_BLOCK            = 0x0080
};


/**
 * Modifier keys
 * 
 * These are the commonly reported modifiers,
 * but additional modifiers are possible
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
enum libterminput_mod {
	/**
	 * Shift modifier
	 */
	LIBTERMINPUT_SHIFT = 0x01,

	/**
	 * Meta/Alternative modifier
	 */
	LIBTERMINPUT_META  = 0x02,

	/**
	 * Control modifier
	 */
	LIBTERMINPUT_CTRL  = 0x04
};


/**
 * Keyboard buttons
 * 
 * Only listed values can be reported, however the value
 * must be listed for the version the application is linked
 * against, not compiled against, so other values can be
 * reported the application is linked against a newer version
 * of the library than it is compiled against
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
enum libterminput_key {
	/**
	 * Non-special key
	 * 
	 * Each code point that is generated as a keypress
	 * is reported separately, meaning that keypresses
	 * that generate multiple code points appear as
	 * multiple keypresses
	 */
	LIBTERMINPUT_SYMBOL,

	LIBTERMINPUT_UP,
	LIBTERMINPUT_DOWN,
	LIBTERMINPUT_RIGHT,
	LIBTERMINPUT_LEFT,
	LIBTERMINPUT_BEGIN,   /* keypad 5 without numlock */
	LIBTERMINPUT_TAB,     /* backtab is interpreted as shift+tab by default */
	LIBTERMINPUT_BACKTAB, /* requires LIBTERMINPUT_SEPARATE_BACKTAB */
	LIBTERMINPUT_F1,
	LIBTERMINPUT_F2,
	LIBTERMINPUT_F3,
	LIBTERMINPUT_F4,
	LIBTERMINPUT_F5,
	LIBTERMINPUT_F6,
	LIBTERMINPUT_F7,
	LIBTERMINPUT_F8,
	LIBTERMINPUT_F9,
	LIBTERMINPUT_F10,
	LIBTERMINPUT_F11,
	LIBTERMINPUT_F12,
	LIBTERMINPUT_HOME,  /* = find */
	LIBTERMINPUT_INS,
	LIBTERMINPUT_DEL,   /* = remove */
	LIBTERMINPUT_END,   /* = select */
	LIBTERMINPUT_PRIOR, /* page up   */
	LIBTERMINPUT_NEXT,  /* page down */
	LIBTERMINPUT_ERASE, /* backspace */
	LIBTERMINPUT_ENTER, /* return    */
	LIBTERMINPUT_ESC,
	LIBTERMINPUT_MACRO,
	LIBTERMINPUT_PAUSE,
	LIBTERMINPUT_KEYPAD_0,
	LIBTERMINPUT_KEYPAD_1,
	LIBTERMINPUT_KEYPAD_2,
	LIBTERMINPUT_KEYPAD_3,
	LIBTERMINPUT_KEYPAD_4,
	LIBTERMINPUT_KEYPAD_5,
	LIBTERMINPUT_KEYPAD_6,
	LIBTERMINPUT_KEYPAD_7,
	LIBTERMINPUT_KEYPAD_8,
	LIBTERMINPUT_KEYPAD_9,
	LIBTERMINPUT_KEYPAD_PLUS,
	LIBTERMINPUT_KEYPAD_MINUS,
	LIBTERMINPUT_KEYPAD_TIMES,
	LIBTERMINPUT_KEYPAD_DIVISION,
	LIBTERMINPUT_KEYPAD_DECIMAL,
	LIBTERMINPUT_KEYPAD_COMMA,
	LIBTERMINPUT_KEYPAD_POINT,
	LIBTERMINPUT_KEYPAD_ENTER

#define LIBTERMINPUT_KEYPAD__LAST__ LIBTERMINPUT_KEYPAD_ENTER /* for internal use */
};


/**
 * Mouse buttons
 * 
 * It is possible that non-listed buttons are
 * reported in events
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
enum libterminput_button {
	/**
	 * No mouse button is held down
	 */
	LIBTERMINPUT_NO_BUTTON,

	/**
	 * Primary button
	 * 
	 * Left button if right-handed,
	 * right button if left-handed
	 */
	LIBTERMINPUT_BUTTON1,

	/**
	 * Middle button
	 */
	LIBTERMINPUT_BUTTON2,

	/**
	 * Secondary button
	 * 
	 * Right button if right-handed,
	 * left button if left-handed
	 */
	LIBTERMINPUT_BUTTON3,

	/**
	 * Wheel scrolled up
	 * 
	 * No corresponding release event shall be generated
	 */
	LIBTERMINPUT_SCROLL_UP,

	/**
	 * Wheel scrolled down
	 * 
	 * No corresponding release event shall be generated
	 */
	LIBTERMINPUT_SCROLL_DOWN,

	/**
	 * Left-scroll button or wheel scrolled left
	 *
	 * May or may not have a corresponding release event
	 */
	LIBTERMINPUT_SCROLL_LEFT,

	/**
	 * Right-scroll button or wheel scrolled right
	 *
	 * May or may not have a corresponding release event
	 */
	LIBTERMINPUT_SCROLL_RIGHT,

	/**
	 * Extended button 1, also known as backward
	 */
	LIBTERMINPUT_XBUTTON1,

	/**
	 * Extended button 2, also known as farward
	 */
	LIBTERMINPUT_XBUTTON2,

	/**
	 * Extended button 3
	 * 
	 * You probably don't have this button
	 */
	LIBTERMINPUT_XBUTTON3,

	/**
	 * Extended button 4
	 * 
	 * You probably don't have this button
	 */
	LIBTERMINPUT_XBUTTON4
};


/**
 * Input event type
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
enum libterminput_type {
	/**
	 * A special value to mark that the input was either
	 * discard or not yet completed
	 */
	LIBTERMINPUT_NONE,

	/**
	 * Normal key press
	 */
	LIBTERMINPUT_KEYPRESS,

	/**
	 * Pseudo-event that marks that beginning of a bracketed paste
	 */
	LIBTERMINPUT_BRACKETED_PASTE_START,

	/**
	 * Pseudo-event that marks that end of a bracketed paste
	 */
	LIBTERMINPUT_BRACKETED_PASTE_END,

	/**
	 * Bracketed paste
	 */
	LIBTERMINPUT_TEXT,

	/**
	 * Mouse event
	 */
	LIBTERMINPUT_MOUSEEVENT,

	/**
	 * OK response for a device status query
	 */
	LIBTERMINPUT_TERMINAL_IS_OK,     /* response to CSI 5 n */

	/**
	 * Not-OK response for a device status query
	 */
	LIBTERMINPUT_TERMINAL_IS_NOT_OK, /* response to CSI 5 n */

	/**
	 * Cursor position report event as a response
	 * to a cursor position query
	 */
	LIBTERMINPUT_CURSOR_POSITION     /* response to CSI 6 n */
};


/**
 * Mouse event subtype
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
enum libterminput_event {
	/**
	 * Mouse button pressed
	 */
	LIBTERMINPUT_PRESS,

	/**
	 * Mouse button released
	 */
	LIBTERMINPUT_RELEASE,

	/**
	 * Mouse moved, possibly with dragging
	 */
	LIBTERMINPUT_MOTION,

	/**
	 * Highlight ended inside of selected region
	 */
	LIBTERMINPUT_HIGHLIGHT_INSIDE,

	/**
	 * Highlight ended outside of selected region
	 */
	LIBTERMINPUT_HIGHLIGHT_OUTSIDE
};


/**
 * Keypress event
 *
 * Some key presses may actually be mouse events,
 * particularly when mouse tracking is disabled.
 * In particular, mouse scrolling may appear as
 * repeated Up key or Down key pesses
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
struct libterminput_keypress {
	/**
	 * Should be `LIBTERMINPUT_KEYPRESS`
	 */
	enum libterminput_type type;

	/**
	 * Which key was pressed
	 */
	enum libterminput_key key;

	/**
	 * This number of types the key was
	 * pressed
	 *
	 * Normally this would be 1, some mouse
	 * events generate arrow key presses that
	 * are reported, not multiple times, but
	 * as clicked multiple times
	 * 
	 * For the next `.times - 1` reads (if
	 * this value is not modified by the user)
	 * will reported as the same event except
	 * that this value will be decreased by 1
	 * each time
	 */
	unsigned long long int times;

	/**
	 * OR of active modifier keys
	 */
	enum libterminput_mod mods;

	/**
	 * The symbol generated by the pressed key
	 * 
	 * Only set if `.key == LIBTERMINPUT_SYMBOL`
	 */
	char symbol[7];
};


/**
 * Text from a bracketed paste
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
struct libterminput_text {
	/**
	 * Should be `LIBTERMINPUT_TEXT`
	 */
	enum libterminput_type type;

	/**
	 * The number of bytes available in `.bytes`
	 */
	size_t nbytes;

	/**
	 * The section of the paste included in this
	 * event report
	 * 
	 * If the text is longer than this buffer, it
	 * is split into multiple events, however they
	 * will all be between the same
	 * `LIBTERMINPUT_BRACKETED_PASTE_START` and
	 * `LIBTERMINPUT_BRACKETED_PASTE_END`, so it is
	 * possible to determine which events are actually
	 * the same paste event
	 */
	char bytes[512];
};


/**
 * Mouse event
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
struct libterminput_mouseevent {
	/**
	 * Should be `LIBTERMINPUT_MOUSEEVENT`
	 */
	enum libterminput_type type;

	/**
	 * Active modifier keys
	 * 
	 * Set to 0 for `LIBTERMINPUT_HIGHLIGHT_INSIDE`
	 * and `LIBTERMINPUT_HIGHLIGHT_OUTSIDE`
	 */
	enum libterminput_mod mods;

	/**
	 * The mouse button used in the event
	 * 
	 * Set to 1 (LIBTERMINPUT_BUTTON1) for
	 * `LIBTERMINPUT_HIGHLIGHT_INSIDE` and
	 * `LIBTERMINPUT_HIGHLIGHT_OUTSIDE`
	 */
	enum libterminput_button button;

	/**
	 * Mouse event sub type
	 */
	enum libterminput_event event;

	/**
	 * Horizontal pointer position
	 * 
	 * The number of cells offset right from the left edge, plus 1
	 */
	size_t x;

	/**
	 * Vertical pointer position
	 * 
	 * The number of cells offset down from the top edge, plus 1
	 */
	size_t y;

	/**
	 * Horizontal beginning of the selection region
	 * 
	 * Only set for `LIBTERMINPUT_HIGHLIGHT_OUTSIDE`
	 */
	size_t start_x;

	/**
	 * Vertical beginning of the selection region
	 * 
	 * Only set for `LIBTERMINPUT_HIGHLIGHT_OUTSIDE`
	 */
	size_t start_y;

	/**
	 * Horizontal end of the selection region
	 * 
	 * Only set for `LIBTERMINPUT_HIGHLIGHT_OUTSIDE`
	 */
	size_t end_x;

	/**
	 * Vertical end of the selection region
	 * 
	 * Only set for `LIBTERMINPUT_HIGHLIGHT_OUTSIDE`
	 */
	size_t end_y;
};


/**
 * Cursor position response
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
struct libterminput_position {
	/**
	 * Should be `LIBTERMINPUT_CURSOR_POSITION`
	 */
	enum libterminput_type type;

	/**
	 * Horizontal cursor position
	 * 
	 * The number of cells offset right from the left edge, plus 1
	 */
	size_t x;

	/**
	 * Vertical cursor position
	 * 
	 * The number of cells offset down from the top edge, plus 1
	 */
	size_t y;
};


/**
 * Input event
 * 
 * @since  1.0 (applies to all members unless stated otherwise)
 */
union libterminput_input {
	/**
	 * Input event type, used to determine which
	 * other member to read
	 * 
	 * The following values have no corresponding
	 * member to read data from:
	 * `LIBTERMINPUT_NONE`,
	 * `LIBTERMINPUT_BRACKETED_PASTE_START`,
	 * `LIBTERMINPUT_BRACKETED_PASTE_END`,
	 * `LIBTERMINPUT_TERMINAL_IS_OK`,
	 * `LIBTERMINPUT_TERMINAL_IS_NOT_OK`
	 * 
	 * Internal comment:
	 * When `.type == LIBTERMINPUT_NONE`, `.keypress.key`
	 * is normally set to `LIBTERMINPUT_SYMBOL`, however
	 * if it is set to anything else, there is a queued
	 * keypress
	 */
	enum libterminput_type type;

	/**
	 * Use if `.type == LIBTERMINPUT_KEYPRESS`
	 */
	struct libterminput_keypress keypress;

	/**
	 * Use if `.type == LIBTERMINPUT_TEXT`
	 */
	struct libterminput_text text;

	/**
	 * Use if `.type == LIBTERMINPUT_MOUSEEVENT`
	 */
	struct libterminput_mouseevent mouseevent;

	/**
	 * Use if `.type == LIBTERMINPUT_CURSOR_POSITION`
	 */
	struct libterminput_position position;
};


/**
 * The current input state and configurations
 * 
 * NB! This struct should be considered opaque
 * 
 * Initialised with by setting all bytes to 0
 * 
 * @since  1.0
 */
struct libterminput_state {
	int inited;                     /* whether the input in initialised, not this struct */
	enum libterminput_mod mods;     /* currently active modifier keys */
	enum libterminput_flags flags;  /* applied behaviour flags */
	unsigned char bracketed_paste;  /* 1 if in bracketed paste, 0 otherwise */
	unsigned char mouse_tracking;   /* length of mouse tracking data, 0 if not processing an mouse tracking event,
	                                 * 1 if undetermined */
	unsigned char meta;             /* number of captured meta/escape bytes */
	unsigned char n;                /* bytes length of partially read character */
	size_t stored_head;             /* index of first available byte in `.stored` */
	size_t stored_tail;             /* index of next byte to write in `.stored` */
	unsigned char paused : 1;       /* 1 if the available buffered input is incomplete */
	unsigned char blocked : 1;      /* 1 if the available no data was available for read(2) */
	unsigned char queued : 1;       /* 1 if a keypress is queued for output if there is no more data at next read(2) */
	unsigned char unused_bits : 5;  /* should be zero due to how the struct should be initialised */
	unsigned char npartial;         /* number of bytes available in `.partial` */
	char partial[7];                /* partially read character */
	char key[44];                   /* processed bytes the identify the pressed key or non-key-press event */
	char stored[512];               /* buffered input not yet processed */
};


/**
 * User provided function and associated binary
 * data used to store the state
 * 
 * @since  1.1 (applies to all members unless stated otherwise)
 */
struct libterminput_marshaller {
	/**
	 * Callback function to store data
	 * 
	 * @param   this  Pointer to the object containing this function
	 * @param   data  The data to store
	 * @param   size  The number of bytes in `data`
	 * @return        0 on success, -1 on failure
	 */
	int (*store)(struct libterminput_marshaller *this, const void *data, size_t size);

	/**
	 * User-defined data
	 */
	union {
		void *ptr;
		int i;
		size_t zu;
	} user;
};


/**
 * User provided function and associated binary
 * data used to load the state
 * 
 * @since  1.1 (applies to all members unless stated otherwise)
 */
struct libterminput_unmarshaller {
	/**
	 * Callback function to load data
	 * 
	 * @param   this  Pointer to the object containing this function
	 * @param   data  Output buffer for the data
	 * @param   size  The number of bytes to load into `data`
	 * @return        0 on success, -1 on failure
	 */
	int (*load)(struct libterminput_unmarshaller *this, void *data, size_t size);

	/**
	 * User-defined data
	 */
	union {
		void *ptr;
		int i;
		size_t zu;
	} user;
};


/**
 * Initialises the parser
 * 
 * Calling this function is optional
 * 
 * Before calling this function, it is important to call
 * `memset(ctx, 0, sizeof(*ctx))` so that `ctx` is properly
 * marked as fully uninitialised (this is needed because
 * the execution of the function is resumed by calling it
 * again after EINTR or EAGAIN failure)
 * 
 * In the current version, this function doesn't do anything,
 * however future versions (or other implementations) may
 * read to and write from the terminal, read the process's
 * environment variables, or read from the filesystem in
 * order to detect the particular quirks of the terminal
 * 
 * @param   ctx  Parser state
 * @param   fd   The file descriptor to the terminal
 * @return       0 on success, -1 on failure
 * 
 * It is unspecified which errors will cause this function
 * to fail, however it should avoid failing for any other
 * reason than EINTR and EAGAIN
 * 
 * @since  1.1
 */
int libterminput_init(struct libterminput_state *ctx, int fd);

/**
 * Deallocate any resources allocated by `libterminput_init`
 * 
 * `ctx` is no longer usable after this function returns
 * 
 * @param  ctx  Parser state
 * 
 * @since  1.1
 */
void libterminput_destroy(struct libterminput_state *ctx);

/**
 * Get input from the terminal
 * 
 * @param   fd     The file descriptor to the terminal
 * @param   input  Output parameter for input
 * @param   ctx    State for the terminal, parts of the state may be stored in `input`
 * @return         1 normally, 0 on end of input, -1 on error
 * 
 * @throws  Any reason specified for read(3)
 * 
 * @since  1.0
 */
int libterminput_read(int fd, union libterminput_input *input, struct libterminput_state *ctx);

/**
 * Check if more input available that can be processed
 * by `libterminput_read` without performing any additional
 * read(3) operation
 * 
 * This function should only be used if using blocking
 * read(3) operations. The flag LIBTERMINPUT_ESC_ON_BLOCK
 * is only meaningful for non-blocking read. With non-blocking
 * read, the application should call libterminput_read(3)
 * and check for EAGAIN error rather than using this function,
 * as EAGAIN errors on read can cause libterminput_read(3) to
 * output keypresses it would otherwise not output, but in
 * doing so not report EAGAIN.
 * 
 * @param   input  Input gathered by `libterminput_read`
 * @param   ctx    State for the terminal
 * @return         1 if there there is more input available, 0 otherwise
 * 
 * @since  1.0
 */
inline int
libterminput_is_ready(const union libterminput_input *input, const struct libterminput_state *ctx)
{
	if (!ctx->inited || ctx->paused || ctx->mouse_tracking)
		return 0;
	if (input->type == LIBTERMINPUT_KEYPRESS && input->keypress.times > 1)
		return 1;
	return ctx->stored_head > ctx->stored_tail;
}

/**
 * Set a behavioural flags
 * 
 * @param   ctx    Argument pasted as the last parameter to `libterminput_read`
 * @param   flags  The OR of the flags to set
 * @return         0 on success, -1 on failure
 * 
 * The current version of this function cannot fail
 * 
 * @since  1.0
 */
int libterminput_set_flags(struct libterminput_state *ctx, enum libterminput_flags flags);

/**
 * Clear a behavioural flags
 * 
 * @param   ctx    Argument pasted as the last parameter to `libterminput_read`
 * @param   flags  The OR of the flags to clear
 * @return         0 on success, -1 on failure
 * 
 * The current version of this function cannot fail
 * 
 * @since  1.0
 */
int libterminput_clear_flags(struct libterminput_state *ctx, enum libterminput_flags flags);

/**
 * Marshal the parsed input
 * 
 * @param   how   Object used to store the serialisation
 * @param   what  The input to marshal
 * @return        0 on success, -1 on failure
 * 
 * This function will fail for any reason `*how->store` fails
 * 
 * @since  1.1
 */
int libterminput_marshal_input(struct libterminput_marshaller *how, const union libterminput_input *what);

/**
 * Marshal the input parsing state
 * 
 * It's important to also use `libterminput_marshal_input`
 * as it may contain part of the state
 * 
 * @param   how   Object used to store the serialisation
 * @param   what  The state to marshal
 * @return        0 on success, -1 on failure
 * 
 * This function will fail for any reason `*how->store` fails
 * 
 * @since  1.1
 */
int libterminput_marshal_state(struct libterminput_marshaller *how, const struct libterminput_state *what);

/**
 * Unmarshal the parsed input
 * 
 * @param   how   Object used to load the serialisation
 * @param   what  Output parameter for the unmarshalled input
 * @return        0 on success, -1 on failure
 * 
 * @throws  EINVAL  Invalid serialisation
 * 
 * This function will also fail for any reason `*how->load` fails
 * 
 * @since  1.1
 */
int libterminput_unmarshal_input(struct libterminput_unmarshaller *how, union libterminput_input *what);

/**
 * Unmarshal the input parsing state
 * 
 * @param   how   Object used to load the serialisation
 * @param   what  Output parameter for the unmarshalled state
 * @return        0 on success, -1 on failure
 * 
 * @throws  EINVAL  Invalid serialisation
 * 
 * This function will also fail for any reason `*how->load` fails
 * 
 * @since  1.1
 */
int libterminput_unmarshal_state(struct libterminput_unmarshaller *how, struct libterminput_state *what);


#endif
