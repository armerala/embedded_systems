#include <stdio.h>


#include "usbkeyboard.h"


struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

struct usb_keyboard_packet packet;

int main()
{
	int transferred;

	keyboard = openkeyboard(&endpoint_address);

	unsigned char buf[8], oldbuf[8];

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

			printf("%s\n", buf);

		
		strcpy(oldbuf, buf);
		}
	}

	return 0;
}
