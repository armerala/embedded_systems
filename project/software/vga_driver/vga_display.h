#ifndef _VGA_DISPLAY_H #define _VGA_DISPLAY_H

#include <linux/ioctl.h>
#include <linux/types.h>

/* Name/UNI:
 *    Daniel Mesko / dpm2153
 *    Cansu Cabuk / cc4455
 *    Alan Armero / aa3938 
 */

#define VGA_DISPLAY_MAGIC 'q'

#define SPRITE_IDLE 0
#define SPRITE_DUCK 1
#define SPRITE_PUNCH 2
#define SPRITE_KICK 3
#define SPRITE_WALK 4
#define SPRITE_DEAD 5
#define SPRITE_JUMP 6
#define SPRITE_POW 7
#define SPRITE_HEART 8

typedef struct {
	uint32_t pix;
	uint32_t addr;
} vga_display_load_arg__t;

typedef struct {
	char magic;
	unsigned short int x;
	unsigned short int y;
	char flags;
} vga_display_render_arg_t;

/* ioctls and their arguments */
#define VGA_DISPLAY_WRITE_SPRITE _IOW(VGA_DISPLAY_MAGIC, 1, vga_display_render_arg_t *)
#define VGA_DISPLAY_LOAD_PIXEL _IOW(VGA_DISPLAY_MAGIC, 1, vga_display_load_arg_t *)


#endif
