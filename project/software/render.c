#include "render.h"
#include "sprite_loader.h"

int vga_display_fd;


void place_sprite(const vga_display_render_t *arg)
{
	vga_display_arg_t vla;
	vla.render = *arg;
	if (ioctl(vga_display_fd, VGA_DISPLAY_WRITE_SPRITE, &vla)) {
		perror("ioctl for ball position failed");
		return;
	}


}
int init_render() 
{
 	static const char filename[] = "/dev/fpga";

  	if ( (vga_display_fd = open(filename, O_RDWR)) == -1) {
    	fprintf(stderr, "could not open %s\n", filename);
    	return -1;
 	 }
	 return 0;

	// load sprites into memory via load-sprite ioctls
	load_sprites(vga_display_fd);

}


void render_frame()
{
	iter_scene(&__do_render);
}

void __do_render(struct scene_object *obj)
{
	struct player_state *state = obj->state;
	
	vga_display_render_t render_obj;
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
