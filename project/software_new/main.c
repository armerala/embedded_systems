#include <signal.h>
#include <stdlib.h>
#include <stdio.h>


#include "input.h"
#include "scene.h"


static int game_done = 0;

static void sig_term_handler(int sig_num)
{
	fprintf(stderr, "CTRL-C caught - press a joystick button to quit\n");
	game_done = 1;
	exit(1);
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

err = 0;
//	err = init_input();
	err = init_scene() | err;
	err = init_render() | err;
	
    //if(init_physics() != 0)
	//	exit(1)

	if(err)
		exit(1);

    //the game loop
    while(!game_done)
    {
        update_inputs();
        update_scene();
		render_frame();		
    }

	shutdown_scene();
	shutdown_input();
	shutdown_render();
    return 0;
}
