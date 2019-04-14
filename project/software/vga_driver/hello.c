/*
 * Userspace program that communicates with the vga_ball device driver
 * through ioctls
 *
 * Stephen A. Edwards
 * Columbia University
 */


/* Name/UNI:
 *    Daniel Mesko / dpm2153
 *    Cansu Cabuk / cc4455
 *    Alan Armero / aa3938 
 */


#include <stdio.h>
#include "vga_display.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int vga_ball_fd;

/* Read and print the background color */
void print_background_color() {
  vga_ball_arg_t vla;
  
  if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
      return;
  }
  printf("%02x %02x %02x\n",
	 vla.background.red, vla.background.green, vla.background.blue);
}

/* Set the background color */
void set_background_color(const vga_ball_color_t *c)
{
  vga_ball_arg_t vla;
  vla.background = *c;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_SET_BACKGROUND) failed");
      return;
  }
}

void move_ball(const vga_ball_position *pos)
{
	vga_ball_arg_t vla;
	vla.position = *pos;
	if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BALL, &vla)) {
		perror("ioctl for ball position failed");
		return;
	}
}


int main()
{
  vga_ball_arg_t vla;
  int i;
  static const char filename[] = "/dev/vga_display";

  static const vga_ball_color_t colors[] = {
    { 0xff, 0x00, 0x00 }, /* Red */
    { 0x00, 0xff, 0x00 }, /* Green */
    { 0x00, 0x00, 0xff }, /* Blue */
    { 0xff, 0xff, 0x00 }, /* Yellow */
    { 0x00, 0xff, 0xff }, /* Cyan */
    { 0xff, 0x00, 0xff }, /* Magenta */
    { 0x80, 0x80, 0x80 }, /* Gray */
    { 0x00, 0x00, 0x00 }, /* Black */
    { 0xff, 0xff, 0xff }  /* White */
  };

# define COLORS 9

  printf("VGA ball Userspace program started\n");

  if ( (vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  int dx,dy;

  set_background_color(&colors[5]);
  
  vga_ball_position pos;

  dx = rand() % 2 * 2 -1;
  dy = rand() % 2 * 2 -1;

  srand(time(NULL));

  pos.x = (rand() % (133));
  pos.y = (rand() % 110);
 
  for (;;) {
	pos.x += dx;
	pos.y += dy;

	if (pos.x < 6 || pos.x > 136)
			dx *= -1;
	if (pos.y < 6 || pos.y > 113)
			dy *= -1;

	move_ball(&pos);

    usleep(20000);
  }

  
  printf("VGA BALL Userspace program terminating\n");
  return 0;
}