CFLAGS = -g -Wall

OBJECTS = gamepad.o usbkeyboard.o

TARFILES = Makefile gamepad.c \
	usbkeyboard.h usbkeyboard.c \

lab2 : $(OBJECTS)
	cc $(CFLAGS) -o gamepad $(OBJECTS) -lusb-1.0 -pthread

lab2.tar.gz : $(TARFILES)
	rm -rf lab2
	mkdir lab2
	ln $(TARFILES) lab2
	tar zcf lab2.tar.gz lab2
	rm -rf lab2

gamepad.o: gamepad.c usbkeyboard.h
usbkeyboard.o : usbkeyboard.c usbkeyboard.h

.PHONY : clean
clean :
	rm -rf *.o gamepad
