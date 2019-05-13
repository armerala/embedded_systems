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

  static const char filename[] = "/dev/fpga";

  vga_display_arg_t vla[2];
  vla[0].x = 3;
  vla[0].y = 10;
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


  if ( (vga_display_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

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

  	place_sprite(&vla[0]);
	place_sprite(&vla[1]);


    usleep(20000);

  }



  return 0;
}
