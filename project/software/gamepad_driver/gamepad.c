#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_joystick.h>
#include <SDL_events.h>

#include "gamepad.h"

// function signatures
void init_gamepads();
void shutdown_gamepads();
void wait_event();


// globals
SDL_Joystick *joystick1, *joystick2;
SDL_Event event;


// dummy main for testing
int main()
{
	init_gamepads();
	wait_event();
	shutdown_gamepads();
	return 0;
}

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
		fprintf(stderr, "One or more joysticks not found, please plug in both!");
		exit(1);
	}


	joystick1 = SDL_JoystickOpen(0);
	joystick2 = SDL_JoystickOpen(1);

	SDL_JoystickEventState(SDL_ENABLE);
}

void shutdown_gamepads()
{
	SDL_JoystickClose(joystick1);
	SDL_JoystickClose(joystick2);
}

void wait_event()
{
	while(1){
		
		if (SDL_WaitEvent(&event))
		{
			switch(event.type){
				case SDL_JOYAXISMOTION:
					printf("axis %d for joystick %d, value: %d\n", event.jaxis.axis, event.jaxis.which, event.jaxis.value);
					break;
				case SDL_JOYBUTTONDOWN:
					printf("button %d pressed for joystick %d\n", event.jbutton.button, event.jbutton.which);
					break;
				case SDL_JOYBUTTONUP:
					printf("button %d released for joystick %d\n", event.jbutton.button, event.jbutton.which);
					break;
				case 256:
					fprintf(stderr, "CTRL-C caught, exiting...\n\n");
					return;
				default:
					printf("some other kind of event: %d\n", event.type);
					break;
			}
		}
		else {
			fprintf(stderr, "there was an error waiting for joystick event, exiting...\n");
			exit(1);
		}
	}

	SDL_JoystickClose(joystick1);
}

