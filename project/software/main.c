#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "input.h"
#include "scene.h"

// GLOBAL TODO: 
//		- figure out how to get sprite images onto board??
//			on disk, loaded on boot? HUH

static int game_done = 0;
static void sig_term_handler(int sig_num)
{
	fprintf(stderr, "CTRL-C caught, exiting...\n");
	game_done = 1;
}

int main()
{
	/*setup sigaction for exit signal*/
	struct sigaction new_act, old_act;
	new_act.sa_handler = &sig_term_handler;
	new_act.sa_flags = SA_RESTART;
	sigemptyset(&new_act.sa_mask);
	if(sigaction(SIGINT, &new_act, &old_act) != 0)
	{
		perror("sigaction failed");
		exit(1);
	}
	
	
    /*the big init*/
	int err;

	err = init_input();
	err = init_scene() | err;

    //if(init_physics() != 0)
	//	exit(1)

	if(err)
		exit(1);

    //the game loop
    while(!game_done)
    {
        update_inputs();
        update_scene();
		//render_frame();		
    }

	shutdown_scene();
	shutdown_input();

    return 0;
}
