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
	unsigned char magic; 	//0xfd is load, 0xfe is render
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned int addr;
} vga_display_load_t;

typedef struct {
	unsigned char magic;
	unsigned short int x;
	unsigned short int y;
	char flags;
} vga_display_render_t;


typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char pos_x;
	unsigned char pos_y;
} vga_display_pixel_t;




typedef struct {
	vga_display_load_t load;
	vga_display_render_t render;
	vga_display_pixel_t pixel;
} vga_display_arg_t;


extern void write_sprite(vga_display_render_t *arg);
extern void load_sprite(vga_display_load_t *arg);

/* ioctls and their arguments */
#define VGA_DISPLAY_WRITE_SPRITE _IOW(VGA_DISPLAY_MAGIC, 1, vga_display_arg_t *)
#define VGA_DISPLAY_LOAD_PIXEL _IOW(VGA_DISPLAY_MAGIC, 2, vga_display_arg_t *)



#endif
