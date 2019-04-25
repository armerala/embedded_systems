#ifndef __RENDER_H__
#define __RENDER_H__

//define the magic #s for rendering
#define IDLE
#define DUCK 1
#define PUNCH 2
#define KICK 3
#define WALk 4
#define DEAD 5

struct sprite_data
{
    char x;
    char y;
    char magic;
};

extern void init_render();
extern void do_render();

#endif
