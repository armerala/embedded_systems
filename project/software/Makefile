CC=gcc
LDFLAGS=-pthread `sdl2-config --cflags --libs`
CFLAGS = -c -g -Wall

OBJS=main.o input.o scene.o render.o state.o #gamepad.o

strEEtfight: $(OBJS)
	$(CC) -g -Wall $(OBJS) -o strEEtfight $(LDFLAGS)

main.o: main.c input.o scene.o
	$(CC) -c -g -Wall main.c `sdl2-config --cflags --libs`

input.o: input.c
	$(CC) $(CFLAGS) input.c `sdl2-config --cflags --libs` 

scene.o: scene.c
	$(CC) $(CFLAGS) scene.c

render.o: render.c
	$(CC) $(CFLAGS) render.c

state.o: state.c
	$(CC) $(CFLAGS) state.c

gamepad.o: gamepad_driver/gamepad.c
	gcc -c -g -Wall gamepad_driver/gamepad.c `sdl2-config --cflags --libs`


.PHONY:clean
clean:
	rm *.o strEEtfight
