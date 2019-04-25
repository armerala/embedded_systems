#ifndef __STATE_H__
#define __STATE_H__

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


#endif
