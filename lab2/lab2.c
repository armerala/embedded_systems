/*
 *
 * CSEE 4840 Lab 2 for 2019
 *
 * Name/UNI: 
 *		 Daniel Mesko/dpm2153
 * 	     Alan Armero/aa3938
 * 	     Cansu Cabuk/cc4455
 */
#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "usbkeyboard.h"
#include <pthread.h>
#include <string.h>


/* Update SERVER_HOST to be the IP address of
 * the chat server you are connecting to
 */
/* micro36.ee.columbia.edu */
#define SERVER_HOST "128.59.148.182"
#define SERVER_PORT 42000

#define BUFFER_SIZE 128

/*
 * References:
 *
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */

int sockfd; /* Socket file descriptor */

struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

pthread_t network_thread;
void *network_thread_f(void *);

uint8_t prev_keyset[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t cur_key;

pthread_mutex_t chat_mutex = PTHREAD_MUTEX_INITIALIZER;
int chat_row = 4;
int chat_col = 0;

// converts USB codes / modifiers to ASCII character
char convert_usb(struct usb_keyboard_packet packet)
{
		int i;
		for (i = 0; i < sizeof(prev_keyset); i++) {
				if (prev_keyset[i] != packet.keycode[i])
						cur_key = packet.keycode[i];
		}

		memcpy(prev_keyset, packet.keycode, sizeof(packet.keycode) * sizeof(uint8_t));

		if ((cur_key <= 0x03) || cur_key > sizeof(USB_CODES)-1)
				return 0;

		struct ascii ch = USB_CODES[cur_key];
		if ((packet.modifiers == 0x02) || (packet.modifiers == 0x20))
				return ch.upper;
		else 
				return ch.lower;

}


// Draw to chat window
void draw_chat(char *str)
{
		char c;
		pthread_mutex_lock(&chat_mutex);
		while ((c = *str++) != 0){								

				if (chat_col == 64) {
						chat_row++;
						chat_col = 0;
				}

				// clear and start again at top on overflow
				if (chat_row == 20) {
						chat_row = 4;
						int col;
						for (col = 0; col < 63; col++) {
								fbputchar(' ', 4, col);
								fbputchar(' ', 5, col);
								fbputchar(' ', 6, col);
								fbputchar(' ', 7, col);
								fbputchar(' ', 8, col);
								fbputchar(' ', 9, col);
								fbputchar(' ', 10, col);
								fbputchar(' ', 11, col);
								fbputchar(' ', 12, col);
								fbputchar(' ', 13, col);
								fbputchar(' ', 14, col);
								fbputchar(' ', 15, col);
								fbputchar(' ', 16, col);
								fbputchar(' ', 17, col);
								fbputchar(' ', 18, col);
								fbputchar(' ', 19, col);
						}
				}


				fbputchar(c, chat_row, chat_col);

				chat_col++;
		}
		pthread_mutex_unlock(&chat_mutex);
		chat_row++;
		chat_col = 0;
}

void insert_char(int buf_len, int cursor_pos, char ch, char *buf)
{	
		// calculate current window position
		int window_pos = cursor_pos % 128;
		int col_pos = window_pos % 64;

		int window_num = cursor_pos / 128;

		int i;

		// shift contents of buffer, inserting character at cursor_pos
		for (i = buf_len; i > cursor_pos; i--)
				buf[i] = buf[i-1];
		buf[cursor_pos] = ch;

		// rewrite the contents of buffer to window
		int col;
		for (col=col_pos; col < 64; col++) {
				int index1 = window_num * 128 + col;
				int index2 = window_num * 128 + 64 + col; 

				char ch1 = (index1 > buf_len) ? ' ' : buf[index1];
				char ch2 = (index2 > buf_len) ? ' ' : buf[index2];

				fbputchar(ch1, 21, col);
				fbputchar(ch2, 22, col);

		}
}

void delete_char(int buf_len, int cursor_pos, char *buf)
{
		// calculate current window position
		int window_num = cursor_pos / 128;

		int i;

		// shift contents of buffer, erasing the character at cursor_pos
		for (i = cursor_pos; (i < buf_len) ; i++)
				buf[i] = buf[i+1];

		buf[buf_len] = 0;

		// rewrite the contents of buffer to window
		int col;
		for (col=0; col < 64; col++) {
				int index1 = window_num * 128 + col;
				int index2 = window_num * 128 + 64 + col; 

				char ch1 = (index1 > (buf_len -2)) ? ' ' : buf[index1];
				char ch2 = (index2 > (buf_len -2)) ? ' ' : buf[index2];

				fbputchar(ch1, 21, col);
				fbputchar(ch2, 22, col);

		}
}	

int main()
{
		int err, col;

		struct sockaddr_in serv_addr;

		struct usb_keyboard_packet packet;
		int transferred;

		if ((err = fbopen()) != 0) {
				fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
				exit(1);
		}

		// clear the screen
		memset(framebuffer, 0, fb_finfo.smem_len);


		/* Draw rows of asterisks across the top and bottom of the screen */
		for (col = 0 ; col < 64 ; col++) {
				fbputchar('*', 0, col);
				fbputchar('*', 23, col);
		}

		fbputs("Hello CSEE 4840 World!", 2, 10);

		// Draw horizontal line
		for (col=0; col < 64; col++) {
				fbputchar('-', 20, col);
		}

		/* Open the keyboard */
		if ( (keyboard = openkeyboard(&endpoint_address)) == NULL ) {
				fprintf(stderr, "Did not find a keyboard\n");
				exit(1);
		}

		/* Create a TCP communications socket */
		if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
				fprintf(stderr, "Error: Could not create socket\n");
				exit(1);
		}

		/* Get the server address */
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(SERVER_PORT);
		if ( inet_pton(AF_INET, SERVER_HOST, &serv_addr.sin_addr) <= 0) {
				fprintf(stderr, "Error: Could not convert host IP \"%s\"\n", SERVER_HOST);
				exit(1);
		}

		/* Connect the socket to the server */
		if ( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
				fprintf(stderr, "Error: connect() failed.  Is the server running?\n");
				exit(1);
		}

		/* Start the network thread */
		pthread_create(&network_thread, NULL, network_thread_f, NULL);


		// initialize position variables and buffer
		int buf_len = 0;
		int cursor_pos = 0;
		char buf[1024];

		// clear buffer
		memset(buf, 0, sizeof(buf));

		/* Look for and handle keypresses */
		for (;;) {

				int window_pos = cursor_pos % 128;
				int row_pos = window_pos / 64;
				int col_pos = window_pos % 64;

				fbputchar('_', row_pos + 21, col_pos);
				// wait for key press
				libusb_interrupt_transfer(keyboard, endpoint_address,
								(unsigned char *) &packet, sizeof(packet),
								&transferred, 0);
				if (transferred == sizeof(packet)) {

						// ESC pressed
						if (packet.keycode[0] == 0x29) { 
								break;
						}

						// Backspace pressed
						if ((packet.keycode[0] == 0x2A)) { 
								if (buf_len != 0) {
										cursor_pos--;
										delete_char(buf_len, cursor_pos, buf);
										buf_len--;
								}
								continue;
						}

						// catch left arrow key, rewrite character at cursor
						if (packet.keycode[0] == 0x50) {
								if (cursor_pos != 0) {
										char replace = (cursor_pos == buf_len) ? ' ' : buf[cursor_pos];
										fbputchar(replace, row_pos + 21, col_pos);
										cursor_pos--;
								}
								continue;
						}
						// catch right arrow key, rewrite character at cursor
						if (packet.keycode[0] == 0x4F) {
								if (cursor_pos != buf_len) {
										fbputchar(buf[cursor_pos], row_pos + 21, col_pos);
										cursor_pos++;
								}
								continue;
						}


						// Convert USB code to ASCII character, check if buffer/window size is
						//  at max, if so clear/move down a line, otherwise write char
						char ch = convert_usb(packet);

						// if ENTER was pressed, send buffer to server, chat window, and reset it
						if (ch == '\n') {
								if (buf_len == 0)
										continue;


								write(sockfd, buf, buf_len);			

								// prepend name for my message
								char fin_buf[buf_len + 5];
								char *me = "<ME> ";
								strcpy(fin_buf, me);
								strcat(fin_buf, buf);
								draw_chat(fin_buf);

								// reset buffer and positional variables
								buf_len = 0;
								cursor_pos = 0;
								memset(buf, 0, sizeof(buf));

								// clear user window
								int col;
								for (col = 0; col < 64; col++) {
										fbputchar(' ', 21, col);
										fbputchar(' ', 22, col);
								}
								continue;
						}

						// If buffer is not full and character is valid, write it to screen
						if ((buf_len != (sizeof(buf) - 1)) && ch != 0) {
								insert_char(buf_len, cursor_pos, ch, buf);
								buf_len++;
								cursor_pos++;
						}
						else if (buf_len == (sizeof(buf) -1)) {
								memset(buf, 0, sizeof(buf));
								buf_len = 0;
								cursor_pos = 0;
						}
				}
		}

		/* Terminate the network thread */
		pthread_cancel(network_thread);

		/* Wait for the network thread to finish */
		return 0;
}

void *network_thread_f(void *ignored)
{
		char recvBuf[BUFFER_SIZE];
		int n;

		/* Receive data */
		while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) {
				recvBuf[n] = 0;												
				draw_chat(recvBuf);

		}

		return NULL;
}
