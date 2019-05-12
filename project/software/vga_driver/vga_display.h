#ifndef _VGA_DISPLAY_H
#define _VGA_DISPLAY_H

#include <linux/ioctl.h>


/* Name/UNI:
 *    Daniel Mesko / dpm2153
 *    Cansu Cabuk / cc4455
 *    Alan Armero / aa3938 
 */

typedef struct {
	char magic;
	uint16_t x;
	uint16_t y;
	char flags;
} vga_display_arg_t;

/* ioctls and their arguments */
#define VGA_DISPLAY_WRITE_SPRITE _IOW(VGA_DISPLAY_MAGIC, 1, vga_display_arg_t *)


#endif
