#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_joystick.h>
#include <SDL_events.h>



void using_sdl();


int main()
{

	using_sdl();
	return 0;
}


void using_sdl()
{

	SDL_Joystick *joystick1, *joystick2;
	
	if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
	{
		fprintf(stderr, "Failed to initialize joystick via SDL: %s\n", SDL_GetError());
		exit(1);
	
	}


	int i, num_joy;
	num_joy = SDL_NumJoysticks();

//	if (num_joy != 2){
//		fprintf(stderr, "One or more joysticks not found, please plug in both!");
//		exit(1);
//	}

	for (i=0; i < num_joy; i++) {
		fprintf(stderr, "joystick number: %d\nname: %s\n", i, SDL_JoystickName(i));
	}

	joystick1 = SDL_JoystickOpen(0);
//	joystick2 = SDL_JoystickOpen(1);

	int id1 = SDL_JoystickInstanceID(joystick1);
	SDL_JoystickEventState(SDL_ENABLE);
	SDL_Event event;

	int x = 100;	
	while(1){
		
		while (SDL_PollEvent(&event))
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
					fprintf(stderr, "SIGTERM caught, exiting...\n\n");
					exit(0);
				default:
					printf("some other kind of event: %d\n", event.type);
					break;
			}
			x--;

		}
		if (!x)
			return;
	}

	SDL_JoystickClose(joystick1);
}

