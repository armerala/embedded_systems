#include "render.h"


int vga_display_fd;

int init_render() 
{
 	static const char filename[] = "/dev/fpga";

  	if ( (vga_display_fd = open(filename, O_RDWR)) == -1) {
    	fprintf(stderr, "could not open %s\n", filename);
    	return -1;
 	 }
	 return 0;

}

void place_sprite(const vga_display_arg_t *arg)
{
	vga_display_arg_t vla;
	vla = *arg;
	if (ioctl(vga_display_fd, VGA_DISPLAY_WRITE_SPRITE, &vla)) {
		perror("ioctl for ball position failed");
		return;
	}
}
void render_frame()
{
	iter_scene(&__do_render);
}

void __do_render(struct scene_object *obj)
{
	struct player_state *state = obj->state;
	
	vga_display_arg_t arg;
	arg.magic = obj->sd->magic;
	arg.x = obj->pos.x;
	arg.y = obj->pos.y;
	arg.flags = obj->sd->flags;

	place_sprite(&arg);

	int i;
	int heart_x; 

	if (state->is_p1) 
		heart_x = 5;
	else
		heart_x = 20;
	

	arg.magic = SPRITE_HEART;
	arg.flags = 0;
	arg.y = 5; // TODO: change hardcoded heart y value

	for (i=0; i < state->health; i++)
	{
		arg.x = heart_x;
		heart_x++;
		place_sprite(&arg);	
	}
}
