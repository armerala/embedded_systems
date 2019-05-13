CC=gcc
LDFLAGS=-pthread `sdl2-config --cflags --libs`
CFLAGS = -c -g -Wall

OBJS=main.o input.o scene.o render.o state.o #gamepad.o

strEEtfight: $(OBJS)
	$(CC) -g -Wall $(OBJS) -o strEEtfight $(LDFLAGS)

main.o: main.c input.o scene.o
	$(CC) -c -g -Wall main.c $(LDFLAGS)

input.o: input.c
	$(CC) $(CFLAGS) input.c $(LDFLAGS)

scene.o: scene.c
	$(CC) $(CFLAGS) scene.c $(LDFLAGS)

render.o: render.c
	$(CC) $(CFLAGS) render.c $(LDFLAGS)

state.o: state.c
	$(CC) $(CFLAGS) state.c $(LDFLAGS)


.PHONY:clean
clean:
	rm *.o strEEtfight
