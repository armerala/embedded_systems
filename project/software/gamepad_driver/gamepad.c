// ***********************************************************************
// gamepad.c
//
// 	Software to initialize, handle events from, and shutdown the gamepads
//
//
// *************************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_joystick.h>
#include <SDL_events.h>

#include "gamepad.h"



// globals
SDL_Joystick *joystick1, *joystick2;
struct gamepad_state pad0, pad1;
SDL_Event event;


// dummy main function for testing
/*
int main()
{
	init_gamepads();
	int x = 100;
	struct gamepad_state *p;
	while (x--)
	{
		p = get_gamepad_event();

		char msg[1000];
		if (p->which == 0)
			strcpy(msg, "Gamepad 0");
		else
			strcpy(msg, "Gamepad 1");

		if (p->b0 == BUTTON_UP)
			strcat(msg, "\nButton 0 up");
		else
			strcat(msg, "\nButton 0 down");

		if (p->b1 == BUTTON_UP)
			strcat(msg, "\nButton 1 up");
		else
			strcat(msg, "\nButton 1 down");

		if (p->x == JOY_LEFT)
			strcat(msg, "\nJoystick Left");
		else if (p->x == JOY_RIGHT)
			strcat(msg, "\nJoystick Right");

		if (p->y == JOY_DOWN)
			strcat(msg, "\nJoystick Down");
		else if (p->y == JOY_UP)
			strcat(msg, "\nJoystick Up");

		printf("%s\n\n", msg);
	}
	return 0;
}
*/

// initialize gamepads
void init_gamepads()
{
	if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
	{
		fprintf(stderr, "Failed to initialize joystick via SDL: %s\n", SDL_GetError());
		exit(1);
	}

	int num_joy;
	num_joy = SDL_NumJoysticks();

	if (num_joy != 2){
		fprintf(stderr, "One or more joysticks not found, please plug in both!\n");
		exit(1);
	}

	joystick1 = SDL_JoystickOpen(0);
	joystick2 = SDL_JoystickOpen(1);

	SDL_JoystickEventState(SDL_ENABLE);
	
	pad0.which = 0;
	pad0.b0 = BUTTON_UP;
	pad0.b1 = BUTTON_UP;
	pad0.x = JOY_CENTER;
	pad0.y = JOY_CENTER;

	pad1.which = 1;
	pad1.b0 = BUTTON_UP;
	pad1.b1 = BUTTON_UP;
	pad1.x = JOY_CENTER;
	pad1.y = JOY_CENTER; 
	
}

void shutdown_gamepads()
{
	SDL_JoystickClose(joystick1);
	SDL_JoystickClose(joystick2);
}

struct gamepad_state *get_gamepad_event()
{
	if (SDL_WaitEvent(&event))
	{
		switch(event.type){
			case SDL_JOYAXISMOTION:
				if (event.jaxis.which == 0) {
					if (event.jaxis.axis == 1) {
						if (event.jaxis.value < 0) 
							pad0.x = JOY_LEFT;
						else if (event.jaxis.value == 0)
							pad0.x = JOY_CENTER;
						else
							pad0.x = JOY_RIGHT;
					}
					else {
						if (event.jaxis.value < 0)
							pad0.y = JOY_DOWN;
						else if (event.jaxis.value == 0)
							pad0.y = JOY_CENTER;
						else 
							pad0.y = JOY_UP;
					}
					return &pad0;
				}
				else {
					if (event.jaxis.axis == 1) {
						if (event.jaxis.value < 0) 
							pad1.x = JOY_LEFT;
						else if (event.jaxis.value == 0)
							pad1.x = JOY_CENTER;
						else
							pad1.x = JOY_RIGHT;
					}
					else {
						if (event.jaxis.value < 0)
							pad1.y = JOY_DOWN;
						else if (event.jaxis.value == 0)
							pad1.y = JOY_CENTER;
						else 
							pad1.y = JOY_UP;
					}
					return &pad1;
				}
				break;
			case SDL_JOYBUTTONDOWN:
				if (event.jbutton.which == 0) {
					if (event.jbutton.button == 0)
						pad0.b0 = BUTTON_DOWN;
					else
						pad0.b1 = BUTTON_DOWN;
					return &pad0;
				}
				else {
					if (event.jbutton.button == 0)
						pad1.b0 = BUTTON_DOWN;
					else
						pad1.b1 = BUTTON_DOWN;
					return &pad1;
				}
				break;
			case SDL_JOYBUTTONUP:
				if (event.jbutton.which == 0) {
					if (event.jbutton.button == 0)
						pad0.b0 = BUTTON_UP;
					else
						pad0.b1 = BUTTON_UP;
					return &pad0;
				}
				else {
					if (event.jbutton.button == 0)
						pad1.b0 = BUTTON_UP;
					else
						pad1.b1 = BUTTON_UP;
					return &pad1;
				}

				break;
			case 256:
				fprintf(stderr, "CTRL-C caught, exiting...\n\n");
				shutdown_gamepads();
				exit(0);
			default:
				break;
		}
		return &pad0;
	}
	else {
		fprintf(stderr, "there was an error waiting for joystick event, exiting...\n");
		exit(1);
	}

}

