#include "scene.h"


struct player_state ps1, ps2;



int init_scene()
{
	ps1.x = 4;
	ps1.y = 10;
	
	ps2.x = 15;
	ps2.y = 10;

	ps1.health = 3;
	ps2.health = 3;

	ps1.behavior = IDLE;
	ps2.behavior = IDLE;
	
	return 0;
}


void update_scene()
{


}


