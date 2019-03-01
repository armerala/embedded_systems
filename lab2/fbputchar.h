#ifndef _FBPUTCHAR_H
#  define _FBPUTCHAR_H

#define FBOPEN_DEV -1          /* Couldn't open the device */
#define FBOPEN_FSCREENINFO -2  /* Couldn't read the fixed info */
#define FBOPEN_VSCREENINFO -3  /* Couldn't read the variable info */
#define FBOPEN_MMAP -4         /* Couldn't mmap the framebuffer memory */
#define FBOPEN_BPP -5          /* Unexpected bits-per-pixel */

extern int fbopen(void);
extern void fbputchar(char, int, int);
extern void fbputs(const char *, int, int);

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/fb.h>

#define FBDEV "/dev/fb0"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define BITS_PER_PIXEL 32

struct fb_var_screeninfo fb_vinfo;
struct fb_fix_screeninfo fb_finfo;
unsigned char *framebuffer;
static unsigned char font[];


struct ascii 
{
		char lower;
		char upper;
};

const static struct ascii USB_CODES[] = 
	{ {0,0}, {0,0}, {0,0}, {0,0}, {'a','A'}, {'b','B'}, {'c','C'}, {'d','D'}, 
	{'e','E'}, {'f','F'}, {'g','G'}, {'h','H'}, 
	{'i','I'}, {'j', 'J'}, {'k','K'}, {'l','L'}, 
	{'m','M'}, {'n','N'}, {'o','O'}, {'p','P'}, 
	{'q','Q'}, {'r','R'}, {'s', 'S'}, {'t','T'}, {'u','U'}, {'v','V'}, {'w','W'},
	{'x','X'}, {'y','Y'}, {'z','Z'}, {'1','!'}, {'2','@'},
	{'3','#'}, {'4','$'}, {'5','%'}, {'6','^'}, {'7','&'}, {'8','*'}, {'9','('}, 
	{'0',')'}, {'\n','\n'}, {0,0}, {0,0},
	{0,0}, {' ',' '}, {'-','_'}, { '=','+'}, {'[','{'}, { ']','}'}, {'\\', '|'}, 
	{0,0}, {';',':'}, {'\'','"'}, 
	{'`', '~'}, {',','<'}, {'.','>'}, {'/', '?'} };


#endif
