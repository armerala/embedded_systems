#ifndef __SPRITE_LOADER_H__
#define __SPRITE_LOADER_H__

#include <stdint.h>

struct __attribute__((__packed__)) file_header {
	unsigned char bfType1;		/* File type identifier, bit 1 */
	unsigned char bfType2;		/* File type identifier, bit 2 */
	uint32_t bfSize;			/* File size */
	uint16_t bfReserved1;		/* Unused/reserved, bit 1 */
	uint16_t bfreserved2; 		/* Unused/reserved, bit 2 */
	uint32_t bfOffBits;			/* Offset into file of pixel data */
} ;

struct __attribute__((__packed__)) image_header {
	uint32_t biSize;			/* Header size */
	int		 biWidth;			/* Image width */
	int		 biHeight;			/* Image Height */ 
	uint16_t biPlanes;			/* Number of planes (always 1) */
	uint16_t biBitCount;		/* Bits per pixel */
	uint32_t biCompression;		/* Compression type (0 if none) */
	uint32_t biSizeimage;		/* Size of compressed pixel data */
	int		 biXPelsPerMeter;	/* Preferred horizontal resolution */
	int		 biYPelsPerMeter;	/* Preffered vertical resolution */
	uint32_t biClrUsed;			/* Used color map entries */
	uint32_t biClrImportant;	/* Number of important coors*/
} ;

struct __attribute__((__packed__)) pixel_data { 
	unsigned char b;
	unsigned char g;	
	unsigned char r;
} ;

extern void load_sprites(int vga_display_fd);
extern void read_and_load(const char *filename, int vga_display_fd);


#endif
