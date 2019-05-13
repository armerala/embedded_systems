#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sprite_loader.h"


// Global variables
FILE *in_fp, *out_fp;
char *out_name;
char new_filename[100];

struct file_header fh;
struct image_header ih;



// Error handling function: prints error, cleans up files/pointers, and exits
static void die(char *msg)
{
	if (in_fp)
		fclose(in_fp);
	if (out_fp) {
		fclose(out_fp);
		remove(out_name);
	}

	fprintf(stderr, "ERROR: %s\n\nFor usage information, add --help\n", msg);
	exit(1);
}


/*
 *  Reads file and image headers, checks type, bits-per-pixel, compression,
 *  and planes - dies if necessary.
 *  Copies headers and color table to output file
 */
void parse_and_copy_headers()
{
	char buf[1024];
	unsigned int remainder;

	// Read file and image headers
	if (fread(&fh, 1, sizeof(fh), in_fp) != sizeof(fh)) 
		die("failed to read file");

	if (fread(&ih, 1, sizeof(ih), in_fp) != sizeof(ih))
		die("failed to read file");


	// Check magic identifier 'BM' for BMP files
	if ((fh.bfType1 != 'B') && (fh.bfType2 != 'M'))
		die("file is not in BMP format");

	// Check that image size is not below 40
	if (ih.biSize < 40)
		die("file is not in BMP format");

	// Check that there is only 1 image plane
	if (ih.biPlanes != 1)
		die("number of image planes does not equal 1");

	// Check that bits per pixel is 24
	if (ih.biBitCount != 24)
		die("bits per pixel does not equal 24");

	// Check that the file is not compressed
	if (ih.biCompression != 0)
		die("file is compressed");


	// Copy over color index table to new file (using 1 MB buffer)
	remainder = fh.bfOffBits - ftell(in_fp);
	while (remainder > 0) {
		if (remainder > sizeof(buf)) {
			if (fread(buf, 1, sizeof(buf), in_fp) != sizeof(buf))
				die("failed to read from file");
			if (fwrite(buf, 1, sizeof(buf), out_fp) != sizeof(buf))
				die("failed to write to file");
			remainder -= sizeof(buf);
		}
		else{
			if (fread(buf, 1, remainder, in_fp) != remainder)
				die("failed to read from file");
			if (fwrite(buf, 1, remainder, out_fp) != remainder)
				die("failed to write to file");
			remainder = 0;
		}
	}

	// Both file positions are now at start of pixel data
}


/* 
 * Reads pixel data from input file, negates colors, writes to output file
 * Determines if rows contain padding bytes, and reads/writes them after
 * reading/writing each row's pixel data
 */
void invert_and_copy_pixels()
{
	struct pixel_data pd;
	unsigned int pixel_remainder;
	unsigned int row_length;
	unsigned int row_remainder;
	unsigned short row_padding;
	unsigned int final_remainder;	
	char buf[1024];
	char pad_buf[4] = { 0, 0, 0, 0};


	// Account for padding at the end of rows (multiples of 4 bytes)
	row_length = (ih.biWidth * sizeof(pd));
	row_padding = 4 - ((abs(ih.biWidth * ih.biHeight) * sizeof(pd)) % 4);
	row_remainder = row_length;

	// Set pixels_remainder
	pixel_remainder = abs(ih.biWidth * ih.biHeight);

	while (pixel_remainder > 0) {
		
		// Read the pixels from input file
		if (fread(&pd, 1, sizeof(pd), in_fp) != sizeof(pd))
			die("failed to read pixel data");

		// Negate the colors
		pd.b = 255 - pd.b;
		pd.g = 255 - pd.g;
		pd.r = 255 - pd.r;

		// Write the new pixels to output file
		if (fwrite(&pd, 1, sizeof(pd), out_fp) != sizeof(pd))
			die("failed to write pixel data");

		pixel_remainder--;

		// Decrement row_remainder, add pad bytes if necessary
		if (row_padding != 4) {
			row_remainder -= sizeof(pd);

			if (row_remainder == row_padding) {
				fseek(in_fp, row_padding, SEEK_CUR);
				
				if (fwrite(&pad_buf, 1, row_padding, out_fp) != row_padding)
					die("failed to write padding bytes");
		
				row_remainder = row_length;
			}
		}
	}

	// Input file may contain extra metadata
	// If so, write it to output file
	if (fh.bfSize != ftell(in_fp)) {
		final_remainder = fh.bfSize - ftell(in_fp);
		while (final_remainder > 0) {
			if (final_remainder > sizeof(buf)) {
				if (fread(buf, 1, sizeof(buf), in_fp) != sizeof(buf))
					die("failed to read from file");
				if (fwrite(buf, 1, sizeof(buf), out_fp) != sizeof(buf))
					die("failed to write to file");
				final_remainder -= sizeof(buf);
			}
			else{
				if (fread(buf, 1, final_remainder, in_fp) != final_remainder)
					die("failed to read from file");
				if (fwrite(buf, 1, final_remainder, out_fp) != final_remainder)
					die("failed to write to file");
				final_remainder = 0;
			}
		}
	}

}


int main(int argc, char **argv)
{
	char *ext;

	// Check args
	if (argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: \n%s <input_filename> <output_filename>\n"
				"%s <input_filename>\n\nOr %s --help\n",
				argv[0], argv[0], argv[0]); 
		exit(1);
	}

	// Print help information
	if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "--Help")) {
		fprintf(stderr, "%s\n", help_msg);
		return 0;
	}

	// Open input file
	in_fp = fopen(argv[1], "rb");
	if (!in_fp) {
		die("file not found");
	}

	// Check if output file name is given, otherwise use default,
	// append .bmp if not there
	if (argc == 3) {
		if (!strstr(argv[2], ".bmp")) {
			sprintf(new_filename, "%s.bmp", argv[2]);
			out_name = new_filename;
		}
		else
			out_name = argv[2];
	}
	else {
		ext = strstr(argv[1], ".bmp");
		if (ext == NULL) 
			sprintf(new_filename, "%s-negative", argv[1]);
		else {
			*ext = 0;
			sprintf(new_filename, "%s-negative.BMP", argv[1]);
		}
		out_name = new_filename;
	}

	// Open output file
	out_fp = fopen(out_name, "wb");

	// Do the real work
	parse_and_copy_headers();
	invert_and_copy_pixels();

	// Clean up
	fprintf(stdout, "SUCCESS: Photo negative of %s written to %s\n",
			argv[1], out_name);

	fclose(in_fp);
	fclose(out_fp);

	return 0;
}
