#include "render.h"
#include "sprite_loader.h"


// globals
int vga_display_fd;

static struct sprite *sprites[9];

const char *filenames[9] = 
		{ "sprite_bmp/idle.bmp", "sprite_bmp/duck.bmp", "sprite_bmp/punch.bmp",
		  "sprite_bmp/kick.bmp", "sprite_bmp/walk.bmp", "sprite_bmp/dead.bmp",
		  "sprite_bmp/jump.bmp", "sprite_bmp/pow.bmp", "sprite_bmp/heart.bmp" };


void place_sprite(const struct render_object *arg)
{

	unsigned char x = arg->x;
	unsigned char y = arg->y;
	char magic = arg->magic;
	char flip_bit = arg->flags; 


	
	// will read r,g,b for each pixel, render at x,y with an offset
	// 		if flip_bit, will read them backwards

	// so sprite_loader needs to malloc sprite_data and send
	// 		pointers back to render.c

	// 	need to structure this data somehow

//	vga_display_arg_t vla;
//	vla.render = *arg;
//	if (ioctl(vga_display_fd, VGA_DISPLAY_WRITE_SPRITE, &vla)) {
//		perror("ioctl for ball position failed");
//		return;
//	}

}
int init_render() 
{
 	static const char filename[] = "/dev/vga_display";

  	if ( (vga_display_fd = open(filename, O_RDWR)) == -1) {
    	fprintf(stderr, "could not open %s\n", filename);
    	return -1;
 	 }

	// load sprites into memory
	int i;
	for (i = 0; i < 9; i++) {
		sprites[i] = load_sprites(filenames[i], vga_display_fd);
	}
	return 0;

}

void shutdown_render()
{

	// free the sprites
	int i;
	for (i = 0; i < sizeof(sprites); i++) {
		struct sprite *sp = sprites[i];
		free(sp->pixel_data);
		free(sp);
		fprintf(stderr, "freed sprite and pixel data\n");
	}
}

void render_frame()
{
	iter_scene(&__do_render);
}

void __do_render(struct scene_object *obj)
{
	struct player_state *state = obj->state;
	
	struct render_object render_obj;
	render_obj.magic = obj->sd->magic;
	render_obj.x = obj->pos.x;
	render_obj.y = obj->pos.y;
	render_obj.flags = obj->sd->flags;

	place_sprite(&render_obj);

	int i;
	int heart_x; 

	if (state->is_p1) 
		heart_x = 5;
	else
		heart_x = 20;
	

	render_obj.magic = SPRITE_HEART;
	render_obj.flags = 0;
	render_obj.y = 5; // TODO: change hardcoded heart y value

	for (i=0; i < state->health; i++)
	{
		render_obj.x = heart_x;
		heart_x++;
		place_sprite(&render_obj);	
	}

	render_obj.magic = 0xff;
	place_sprite(&render_obj);

}
