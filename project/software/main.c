#include <stdlib.h>

#include "input.h"
#include "scene.h"

// GLOBAL TODO: 
//		- figure out how to get sprite images onto board??
//			on disk, loaded on boot? HUH



int main()
{
    /*the big init*/
	if(init_input() != 0)
        exit(1);

	if(init_scene() != 0)
		exit(1);

    //init physics

    //the game loop
    while(1)
    {
        update_inputs();
        update_scene();
		//render_frame();		
    }

    return 0;
}
