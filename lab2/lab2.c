/*
 *
 * CSEE 4840 Lab 2 for 2019
 *
 * Name/UNI: Daniel Mesko/dpm2153
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
pthread_mutex_t sock_mutex = PTHREAD_MUTEX_INITIALIZER;
void *network_thread_f(void *);


// converts USB codes / modifiers to ASCII character
char convert_usb(struct usb_keyboard_packet packet)
{
				if ((packet.keycode[0] <= 0x03) || packet.keycode[0] > sizeof(USB_CODES)-1)
								return 0;

				struct ascii ch = USB_CODES[packet.keycode[0]];
				if ((packet.modifiers == 0x02) || (packet.modifiers == 0x20))
								return ch.upper;
				else 
								return ch.lower;

}

void insert_char(int buf_len, int cursor_pos, char ch, char *buf)
{	
				// calculate current window position
				int window_pos = cursor_pos % 128;
//				int row_pos = window_pos / 64;
				int col_pos = window_pos % 64;
			
				int window_num = cursor_pos / 128;

				int i;

				// shift contents buffer, inserting character at cursor_pos
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
				char keystate[12];

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

				fbputs("Hello CSEE 4840 World!", 4, 10);

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

				// TODO: Maybe remove?? (nonblocking flag set for socket)
				int flags = fcntl(sockfd, F_GETFL, 0);
				fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

				/* Start the network thread */
				pthread_create(&network_thread, NULL, network_thread_f, NULL);


				int buf_len = 0;
				int cursor_pos = 0;
				char buf[512];

				/* Look for and handle keypresses */
				for (;;) {

								// TODO:?? Simulatenous keypress issue..previous key registered again before the next key registered...
			
								// wait for key press
								libusb_interrupt_transfer(keyboard, endpoint_address,
																(unsigned char *) &packet, sizeof(packet),
																&transferred, 0);
								if (transferred == sizeof(packet)) {
												sprintf(keystate, "%02x %02x %02x", packet.modifiers, packet.keycode[0],
																				packet.keycode[1]);
												printf("%s\n", keystate);

												if (packet.keycode[0] == 0x29) { /* ESC pressed */
																break;
												}

												if ((packet.keycode[0] == 0x2A)) { /* Backspace pressed */
																if (buf_len != 0) {
																				cursor_pos--;
																				delete_char(buf_len, cursor_pos, buf);
																				buf_len--;
																}
																continue;
												}

												// catch left and right arrow keys
												if (packet.keycode[0] == 0x50) {
																if (cursor_pos != 0)
																				cursor_pos--;
																continue;
												}
												if (packet.keycode[0] == 0x4F) {
																if (cursor_pos != buf_len)
																				cursor_pos++;
																continue;
												}


												// Convert USB code to ASCII character, check if buffer/window size is
												//  at max, if so clear/move down a line, otherwise write char
												char ch = convert_usb(packet);

												// if ENTER was pressed, null terminate the buffer and send to server
												if (ch == '\n') {
																if (buf_len == 0)
																				continue;
																buf[buf_len] = 0;
																buf_len++;
																char *p = buf;
																int n;

																pthread_mutex_lock(&sock_mutex);
																while ( (n = write(sockfd, p, buf_len)) > 0) {
																				p += n;
																				buf_len -= n;
																}
																pthread_mutex_unlock(&sock_mutex);

																buf_len = 0;
																cursor_pos = 0;

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
				int row = 8;

				/* Receive data */
				for (;;) {
								pthread_mutex_lock(&sock_mutex);
								while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) {
												recvBuf[n] = '\0';
												printf("%s", recvBuf);
												fbputs(recvBuf, row, 0);
												row++;
								}
								pthread_mutex_unlock(&sock_mutex);
				}

				printf("did you termintae??\n");
				return NULL;
}

