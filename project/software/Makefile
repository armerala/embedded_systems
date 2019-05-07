CC=gcc
LDFLAGS=-pthread
CFLAGS = -c -g -Wall

OBJS=main.o input.o scene.o render.o state.o

strEEtfight: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o strEEtfight

main.o: main.c input.o scene.o
	$(CC) -c -g -Wall main.c

input.o: input.c
	$(CC) $(CFLAGS) input.c

scene.o: scene.c
	$(CC) $(CFLAGS) scene.c

render.o: render.c
	$(CC) $(CFLAGS) render.c

state.o: state.c
	$(CC) $(CFLAGS) state.c


.PHONY:clean
clean:
	rm *.o strEEtfight
