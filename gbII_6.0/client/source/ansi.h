/*
 * ansi.h
 *
 * ANSI control codes for various neat-o terminal effects
 *
 * Some older version of Ultrix don't appear to be able to handle
 * these escape sequences. If lowercase 'a's are being stripped,
 * and/or the output of the ANSI is screwed up, you have the Ultrix
 * problem.
 *
 * To fix the ANSI problem, try replacing the 'x1B' with '\033'.
 * To fix the problem with 'a's, replace all occurrances of '\a' in the code
 * with '\07'.
 */

#ifndef ANSI_H_
#define ANSI_H_

#define BEEP_CHAR            '\a'
#define ESC_CHAR             '\x1B'

#define ANSI_BEGIN           "\x1B["

#define ANSI_NORMAL          "\x1B[0m"

#define ANSI_HILITE          "\x1B[1m"
#define ANSS_INVERSE         "\x1B[7m"
#define ANSI_BLINK           "\x1B[5m"
#define ANSI_UNDERSCORE      "\x1B[4m"
#define ANSI_TRUNCATE        "\x1B[=71"

#define ANSI_INV_BLINK       "\x1B[7;5m"
#define ANSI_INV_HILITE      "\x1B[1;7m"
#define ANSI_BLINK_HILITE    "\x1B[1;5m"
#define ANSI_INV_BLINK_WHITE "\x1B[1;5;7m"

/* Foreground colors */

#define ANSI_FOR_BLACK       "\x1B[30m"
#define ANSI_FOR_RED         "\x1B[31m"
#define ANSI_FOR_GREEN       "\x1B[32m"
#define ANSI_FOR_YELLOW      "\x1B[33m"
#define ANSI_FOR_BLUE        "\x1B[34m"
#define ANSI_FOR_MAGENTA     "\x1B[35m"
#define ANSI_FOR_CYAN        "\x1B[36m"
#define ANSI_FOR_WHITE       "\x1B[37m"

/* Background colors */

#define ANSI_BAC_BLACK       "\x1B[40m"
#define ANSI_BAC_RED         "\x1B[41m"
#define ANSI_BAC_GREEN       "\x1B[42m"
#define ANSI_BAC_YELLOW      "\x1B[43m"
#define ANSI_BAC_BLUE        "\x1B[44m"
#define ANSI_BAC_MAGENTA     "\x1B[45m"
#define ANSI_BAC_CYAN        "\x1B[46m"
#define ANSI_BAC_WHITE       "\x1B[47m"

#define ANSI_END             "m"

/* HTML tags */

#define TAG_START '\02'
#define TAG_END   '\03'

#define COL_FLASH      (1) /**< ANSI flash attribute bit */
#define COL_HILITE     (2) /**< ANSI hilite attribute bit */
#define COL_INVERT     (4) /**< ANSI inverse attribute bit */
#define COL_UNDERSCORE (8) /**< ANSI underscore attribute bit */

#define VAL_FLASH      (5) /**< ANSI flag attribute value */
#define VAL_HILITE     (1) /**< ANSI hilite attribute value */
#define VAL_INVERT     (7) /**< ANSI inverse attribute value */
#define VAL_UNDERSCORE (4) /**< ANSI underscore attribute value */

#define COL_BLACK     (30) /**< ANSI color black */
#define COL_RED       (31) /**< ANSI color red */
#define COL_GREEN     (32) /**< ANSI color green */
#define COL_YELLOW    (33) /**< ANSI color yellow */
#define COL_BLUE      (34) /**< ANSI color blue */
#define COL_MAGENTA   (35) /**< ANSI color magenta */
#define COL_CYAN      (36) /**< ANSI color cyan */
#define COL_WHITE     (37) /**< ANSI color white */

#define MAX_RCOLORS 7 /* Number of colors used for races */

#endif /* ANSI_H_ */
