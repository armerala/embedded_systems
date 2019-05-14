#ifndef __RENDER_H__
#define __RENDER_H__


#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#include "scene.h"
#include "state.h"
#include "vga_driver/vga_display.h"

//define the magic #s for behavior
#define IDLE 0
#define DUCK 1
#define PUNCH 2
#define KICK 3
#define WALK 4
#define DEAD 5
#define JUMP 6


#define SPRITE_FLIP_X 1


struct sprite_data
{
    char magic;
	char flags; //e.g. flip bit etc.
};

extern int init_render();
extern void place_sprite(const vga_display_render_t *arg);
extern void __do_render(struct scene_object *obj);
extern void render_frame();

#endif
