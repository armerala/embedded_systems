#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>


#include "sprite_loader.h"
#include "vga_driver/vga_display.h"


// Error handling function: prints error, cleans up files/pointers, and exits
static void die(char *msg, FILE * in_fp)
{
	if (in_fp)
		fclose(in_fp);

	fprintf(stderr, "ERROR: %s\n\nFor usage information, add --help\n", msg);
	exit(1);
}



struct sprite *read_and_load(const char *filename, int vga_display_fd)
{
	FILE *in_fp;

	struct file_header fh;
	struct image_header ih;

	// Open input file
	in_fp = fopen(filename, "rb");
	if (!in_fp) 
		die("file not found", in_fp);


	// Read file and image headers
	if (fread(&fh, 1, sizeof(fh), in_fp) != sizeof(fh)) 
		die("failed to read file", in_fp);

	if (fread(&ih, 1, sizeof(ih), in_fp) != sizeof(ih))
		die("failed to read file", in_fp);


	// Check magic identifier 'BM' for BMP files
	if ((fh.bfType1 != 'B') && (fh.bfType2 != 'M'))
		die("file is not in BMP format", in_fp);

	// Check that image size is not below 40
	if (ih.biSize < 40)
		die("file is not in BMP format", in_fp);

	// Check that there is only 1 image plane
	if (ih.biPlanes != 1)
		die("number of image planes does not equal 1", in_fp);

	// Check that bits per pixel is 24
	if (ih.biBitCount != 24)
		die("bits per pixel does not equal 24", in_fp);

	// Check that the file is not compressed
	if (ih.biCompression != 0)
		die("file is compressed", in_fp);


	// seek to pixel data
	if (fseek(in_fp, fh.bfOffBits, SEEK_SET))
		die("could not seek to pixel data offset in sprite file", in_fp);
	

	// file position now at start of pixel data
	struct pixel_data pd;
	unsigned int pixel_remainder;
	unsigned int row_length;
	unsigned int row_remainder;
	unsigned short row_padding;

	unsigned int addr = 0;


	// Account for padding at the end of rows (multiples of 4 bytes)
	row_length = (ih.biWidth * sizeof(pd));
	row_padding = 4 - ((abs(ih.biWidth * ih.biHeight) * sizeof(pd)) % 4);
	row_remainder = row_length;

	// Set pixels_remainder
	pixel_remainder = abs(ih.biWidth * ih.biHeight);

	struct sprite *sprite;
	unsigned char *pixel_data;
	int data_size;

	sprite = (struct sprite *)malloc(sizeof(struct sprite));

	fprintf(stderr, "malloc'd %d for sprite table\n", sizeof(struct sprite));
	sprite->width = ih.biWidth;
	sprite->height = ih.biHeight; 
	
	data_size = ih.biWidth * ih.biHeight * 24;
	
	pixel_data = malloc(data_size);
	fprintf(stderr, "malloc'd %d for pixel data\n", data_size);
	sprite->pixel_data = pixel_data;
		
	unsigned char *iterator = pixel_data;	
	while (pixel_remainder > 0) {
		
		// Read the pixels from input file
		if (fread(&pd, 1, sizeof(pd), in_fp) != sizeof(pd))
			die("failed to read pixel data", in_fp);

		*iterator++ = pd.r;
		*iterator++ = pd.g;
		*iterator++ = pd.b;

		// Decrement row_remainder, add pad bytes if necessary
		if (row_padding != 4) {
			row_remainder -= sizeof(pd);

			if (row_remainder == row_padding) {
				fseek(in_fp, row_padding, SEEK_CUR);
				
				row_remainder = row_length;
			}
		}
	}

	fclose(in_fp);
	return sprite;
}

struct sprite *load_sprites(const char *filename, int vga_display_fd)
{

	read_and_load(filename, vga_display_fd);

}
