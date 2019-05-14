#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


#include "vga_display.h"


int vga_display_fd;

void load_pixel(const vga_display_arg_t *arg)
{
	vga_display_arg_t vla;
	vla = *arg;
	if (ioctl(vga_display_fd, VGA_DISPLAY_LOAD_PIXEL, &vla)) {
		perror("ioctl for ball position failed");
		return;
	}
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


int main()
{
 
  //open the dev
  static const char filename[] = "/dev/fpga";
  if ( (vga_display_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  //load some pixel
  unsigned short int i;
  vga_display_load_t load_arg;
  vga_display_arg_t arg;
  load_arg.r = 152;
  load_arg.g = 99;
  load_arg.b = 69;
  load_arg.magic = 0xfd;
  for(i = 0 ; i < 200000; i++) {
	  load_arg.addr = i;
	  arg.load = load_arg;
      load_pixel(&arg);
  }

  //place some sprites (one is flipped)
  vga_display_render_t vla[2];
  vla[0].x = 0;
  vla[0].y = 0;
  vla[0].magic = SPRITE_IDLE;
  vla[0].flags = 0;


  vla[1].x = 10;
  vla[1].y = 10;
  vla[1].magic = SPRITE_IDLE;
  vla[1].flags = 1;

  int dx0,dy0,dx1,dy1;

  dx0 = rand() % 2 * 2 -1;
  dy0 = rand() % 2 * 2 -1;

  dx1 = rand() % 2 * 2 -1;
  dy1 = rand() % 2 * 2 -1;

  srand(time(NULL));

  for ( ;; )
  {
	
	vla[0].x += dx0;
	vla[0].y += dy0;

	vla[1].x += dx1;
	vla[1].y += dy1;

	if (vla[0].x < 6 || vla[0].x > 136)
			dx0 *= -1;
	if (vla[0].y < 6 || vla[0].y > 113)
			dy0 *= -1;

	if (vla[1].x < 6 || vla[1].x > 136)
			dx1 *= -1;
	if (vla[1].y < 6 || vla[1].y > 113)
			dy1 *= -1;

	arg.render = vla[0];
  	place_sprite(&arg);
	arg.render = vla[1];
	place_sprite(&arg);
	vla[1].magic = 0xff;
	place_sprite(&arg);


    usleep(20000);

  }



  return 0;
}
