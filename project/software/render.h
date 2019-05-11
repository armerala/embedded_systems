#ifndef __RENDER_H__
#define __RENDER_H__

//define the magic #s for rendering
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

extern void init_render();
extern void do_render();

#endif
