#include <stdio.h>
#include <linux/joystick.h>
#include <stdlib.h>
#include <string.h>

#include "usbkeyboard.h"


struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

struct usb_keyboard_packet packet;

void using_libusb();
void using_joystick();


int main()
{

//	using_joystick();

	using_libusb();
	return 0;
}


void using_joystick()
{
		printf("%s\n", JSIOCGVERSION);
}



void using_libusb()
{
	int transferred;

	keyboard = openkeyboard(&endpoint_address);

	unsigned char buf[8] = { 0 };
	unsigned char oldbuf[8] = { 0 };

	for (;;) {

		int ret = libusb_interrupt_transfer(keyboard, endpoint_address, buf, sizeof(buf), &transferred, 0);

		if (transferred == sizeof(buf)) {
			char *err;
			switch(ret) {
				case LIBUSB_ERROR_PIPE:
					err = "endpoint halted";
					break;
				case LIBUSB_ERROR_OVERFLOW:
					err = "overflow";
					break;
				case LIBUSB_ERROR_NO_DEVICE:
					err = "no device";
					break;
				case LIBUSB_ERROR_BUSY:
					err = "busy";
					break;
				case LIBUSB_ERROR_TIMEOUT:
					err = "timeout";
					break;
				case 0:
					err = "no error";
					break;
				default:
					err = "unknown error";
					break;
			}


			if (memcmp(buf, oldbuf, sizeof(buf)) != 0) {
				printf("%lx\n", *(long *)buf);
				memcpy(oldbuf, buf, sizeof(buf));
			}
		}
	}
}
