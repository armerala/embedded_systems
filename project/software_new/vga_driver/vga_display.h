#ifndef _VGA_DISPLAY_H 
#define _VGA_DISPLAY_H

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
	unsigned char p1_x;
	unsigned char p1_y;
	unsigned char p2_x;
	unsigned char p2_y;
	char p1_health;
	char p2_health;
} vga_display_arg_t;


extern void place_players(vga_display_arg_t *arg);


/* ioctls and their arguments */
#define VGA_DISPLAY_PLACE_PLAYERS _IOW(VGA_DISPLAY_MAGIC, 1, vga_display_arg_t *)


#endif
