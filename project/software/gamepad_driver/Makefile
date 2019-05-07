TARFILES = Makefile gamepad.c 

gamepad: gamepad.o
	gcc -g -Wall gamepad.o -o gamepad `sdl2-config --cflags --libs`

gamepad.o: gamepad.c
	gcc -c -g -Wall gamepad.c `sdl2-config --cflags --libs`


lab2.tar.gz : $(TARFILES)
	rm -rf lab2
	mkdir lab2
	ln $(TARFILES) lab2
	tar zcf lab2.tar.gz lab2
	rm -rf lab2


.PHONY : clean
clean :
	rm -rf *.o gamepad
