#include "render.h"


// globals
int vga_display_fd;

static struct sprite *sprites[9];

const char *filenames[9] = 
		{ "sprite_bmp/idle.bmp", "sprite_bmp/duck.bmp", "sprite_bmp/punch.bmp",
		  "sprite_bmp/kick.bmp", "sprite_bmp/walk.bmp", "sprite_bmp/dead.bmp",
		  "sprite_bmp/jump.bmp", "sprite_bmp/pow.bmp", "sprite_bmp/heart.bmp" };


void call_vga(vga_display_arg_t *arg)
{
	vga_display_arg_t vla;
	vla = *arg;

	if (ioctl(vga_display_fd, VGA_DISPLAY_PLACE_PLAYERS, &vla)) {
		perror("ioctl for sprite write failed");
		exit(1);
	}
}

/*
void place_sprite(const struct render_object *arg)
{

	unsigned char x = arg->x;
	unsigned char y = arg->y;
	char magic = arg->magic;
	char flip_bit = arg->flags; 


	struct sprite *sp;
	unsigned char *iterator;
	
	int row_count = 0;
	int column_count = 0;

	sp = sprites[magic];
	iterator = sp->pixel_data;


// TODO: add flip bit logic

	vga_display_pixel_t pixel;
	while (row_count <= sp->height) {
		pixel.r = *iterator++;
		pixel.g = *iterator++;
		pixel.b = *iterator++;
		
		pixel.pos_x = x;
		pixel.pos_x = y;

		call_vga(&pixel);

		x++;
		column_count++;

		if (column_count > sp->width) {
			column_count = 0;
			row_count++;
			y++;
			x = arg->x;
		}

		usleep(10000);	
	}
}
	
*/	




int init_render() 
{
 	static const char filename[] = "/dev/vga_display";

  	if ( (vga_display_fd = open(filename, O_RDWR)) == -1) {
    	fprintf(stderr, "could not open %s\n", filename);
    	return -1;
 	 }

	// load sprites into memory
//	int i;
//	for (i = 0; i < 9; i++) {
//		sprites[i] = load_sprites(filenames[i], vga_display_fd);
//	}
	
	return 0;

}

void shutdown_render()
{

	// free the sprites
	int i;
	for (i = 0; i < 9; i++) {
		struct sprite *sp = sprites[i];
		free(sp->pixel_data);
		free(sp);
	}
}

void render_frame()
{
	iter_scene(&__do_render);
}


void __do_render(struct scene_object *obj)
{
	
	struct player_state *state = obj->state;
	if (!(state->is_p1))
		return;
	
	struct scene_object *other_obj = obj->other;
	struct player_state *other_state = other_obj->state;


	vga_display_arg_t arg;
	arg.p1_x = (unsigned char) (truncf(obj->pos.x));
	arg.p1_y = (unsigned char) (truncf(obj->pos.y));
	arg.p2_x = (unsigned char) (truncf(other_obj->pos.x));
	arg.p2_y = (unsigned char) (truncf(other_obj->pos.y));
	arg.p1_health = state->health;
	arg.p2_health = other_state->health;

	call_vga(&arg);

	float f, dummy_pos;
	if (obj->sd->magic == JUMP) {
		dummy_pos = arg.p1_y;

		for (f = 9.0; f >= 0; f-=0.0001) {
			 dummy_pos -= 0.0001;
			 arg.p1_y = (unsigned char) (truncf(dummy_pos));
			 call_vga(&arg);
		}

		for (f = 0; f <= 9.0; f+=0.0001) {
			 dummy_pos += 0.0001;
			 arg.p1_y = (unsigned char) (truncf(dummy_pos));
			 call_vga(&arg);
		}

		obj->sd->magic = IDLE;


	}
	if (other_obj->sd->magic == JUMP) {
		dummy_pos = arg.p2_y;

		for (f = 9.0; f > 0; f-=0.0001) {
			 dummy_pos -= 0.0001;
			 arg.p2_y = (unsigned char) (truncf(dummy_pos));
			 call_vga(&arg);
		}

		for (f = 0; f < 9.0; f+=0.0001) {
			 dummy_pos += 0.0001;
			 arg.p2_y = (unsigned char) (truncf(dummy_pos));
			 call_vga(&arg);
		}

		other_obj->sd->magic = IDLE;
	}

	

}
