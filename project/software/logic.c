// GLOBAL TODO: 
//		- figure out how to get sprite images onto board??
//			on disk, loaded on boot? HUH





#include <stdio.h>
#include <pthread.h>


 // TODO: move to header file

#define IDLE 0
#define DUCK 1
#define PUNCH 2
#define KICK 3
#define WALk 4
#define DEAD 5



struct player_state
{
	char x;
	char y;
	char health; // 0-3
	char behavior;
};


struct player_args
{
	pthread_mutex_t lock;
	struct player_state state;
	char set;
};


// thread objects
pthread_t thread_1, thread_2;
void *handle_input(void *arg);

// global state objects
struct player_args player1, player2;


int main()
{
	int ret1, ret2;

	// initialize mutexes
	if (pthread_mutex_init(&player1.lock, NULL) != 0){
		fprintf(stderr, "failed to initialize mutex\n");
		return 1;
	}
	if (pthread_mutex_init(&player2.lock, NULL) != 0){
		fprintf(stderr, "failed to initialize mutex\n");
		return 1;
	}


	ret1 = pthread_create(&thread_1, NULL, handle_input, &player1);
	ret2 = pthread_create(&thread_2, NULL, handle_input, &player2);

	// wait on change

	// TODO: Need to mutex when reading player state
	if (player1.set){

		pthread_mutex_lock(&player1.lock);
		// update game state
		// call device drivers to update blablabla
		player1.set = 0;
		pthread_mutex_unlock(&player1.lock);
	}
	if (player2.set){
			pthread_mutex_lock(&player2.lock);
			player2.set = 0;
			pthread_mutex_unlock(&player2.lock);
	}
		

	return 0;
}

void *handle_input(void *arg)
{

	for (;;)
	{
		// TODO: blocking joystick input call

		// based on what it returned, update arg->state, then set arg->set

	}
}


