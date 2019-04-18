CC=gcc
LDFLAGS=-pthread
CFLAGS = -c -g -Wall

OBJS=main.o input.o scene.o

strEEtfight: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o strEEtfight

main.o: main.c input.o scene.o
	$(CC) -c -g -Wall main.c

input.o: input.c
	$(CC) $(CFLAGS) input.c

scene.o: scene.c
	$(CC) $(CFLAGS) scene.c


.PHONY:clean
clean:
	rm *.o strEEtfight
