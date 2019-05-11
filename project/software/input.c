#include "input.h"

//workers/threading
static pthread_t input_thread;
static pthread_mutex_t p1_input_lock;
static pthread_mutex_t p2_input_lock;
static uint8_t work_done = 0;

//SDL data
static SDL_Joystick *joystick1, *joystick2;

//input bitmasks
static uint8_t p1_input_bitmask;
static uint8_t p1_input_bitmask_prev;
static uint8_t p1_input_bitmask_down;
static uint8_t p1_input_bitmask_up;

static uint8_t p2_input_bitmask;
static uint8_t p2_input_bitmask_prev;
static uint8_t p2_input_bitmask_down;
static uint8_t p2_input_bitmask_up;

//axes values
static uint16_t p1_x_axis;
static uint16_t p1_x_axis_prev;
static uint16_t p1_y_axis;
static uint16_t p1_y_axis_prev;

static uint16_t p2_x_axis;
static uint16_t p2_x_axis_prev;
static uint16_t p2_y_axis;
static uint16_t p2_y_axis_prev;


/**
 * return 0 on success, anything else is failure
 */
int init_input() 
{
	int ret;

	//worker init threads
	ret = pthread_mutex_init(&p2_input_lock, NULL);
	ret |= pthread_mutex_init(&p1_input_lock, NULL);
	if (ret != 0){
	    fprintf(stderr, "failed to initialize mutex\n");
		work_done = 1;
	    return 1;
	}


	if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
	{
		fprintf(stderr, "Failed to initialize joystick via SDL: %s\n", SDL_GetError());
		return 1;

	}

	int num_joy;
	num_joy = SDL_NumJoysticks();

	if (num_joy != 2){
		fprintf(stderr, "One or more joysticks not found, please plug in both!\n");
		return 1;
	}

	//init sdl
	joystick1 = SDL_JoystickOpen(0);
	joystick2 = SDL_JoystickOpen(1);


	//create sdl worker
	ret = pthread_create(&input_thread, NULL, __handle_joy, NULL);
	if(ret != 0){
		fprintf(stderr, "Failed to initialize joystick via SDL: %s\n", SDL_GetError());
		work_done = 1;
		return 1;
	}

	return 0;
}

void shutdown_input() 
{
	work_done = 1;
	pthread_join(input_thread, NULL);
}

/**
 * Called once per frame at the beginning of the frame to update
 * the button-up/down for this frame
 */
void update_inputs()
{
    pthread_mutex_lock(&p1_input_lock);
    pthread_mutex_lock(&p2_input_lock);

    p1_input_bitmask_down = p1_input_bitmask & ~p1_input_bitmask_prev;
    p1_input_bitmask_up = ~p1_input_bitmask & p1_input_bitmask_prev;
    p1_input_bitmask_prev = p1_input_bitmask;
	p1_x_axis_prev = p1_x_axis;
	p1_y_axis_prev = p1_y_axis;

    p2_input_bitmask_down = p2_input_bitmask & ~p2_input_bitmask_prev;
    p2_input_bitmask_up = ~p2_input_bitmask & p2_input_bitmask_prev;
    p2_input_bitmask_prev = p2_input_bitmask;
	p2_x_axis_prev = p2_x_axis;
	p2_y_axis_prev = p2_y_axis;

    pthread_mutex_unlock(&p2_input_lock);
    pthread_mutex_unlock(&p1_input_lock);
}

/**
 * used to retrieve whether a button went down this frame
 */
int get_button_down(int keycode, int is_p1)
{
    if(is_p1)
        return p1_input_bitmask_down & keycode;
    else
        return p2_input_bitmask_down & keycode;
}

/**
 * Used to return whether a button went up this frame
 */
int get_button_up(int keycode, int is_p1)
{
    if(is_p1)
        return p1_input_bitmask_up & keycode;
    else
        return p2_input_bitmask_up & keycode;
}

/**
 * get if button is currently down (regardless of up or down this frame)
 */
int get_button(int keycode, int is_p1)
{
    if(is_p1)
        return p1_input_bitmask_prev & keycode;
    else
        return p2_input_bitmask_prev & keycode;
}

int get_axis(int axis, int is_p1)
{
	if(is_p1)
		return (axis == 0) ? p1_x_axis_prev : p1_y_axis_prev;
	else
		return (axis == 0) ? p2_y_axis_prev : p2_y_axis_prev;
}

/**
 * A worker for handling input in the background
 */
void* __handle_joy(void* arg)
{
	int p1_joy_id;
	int is_p1;
	
	uint16_t* axis_val;

	p1_joy_id = SDL_JoystickInstanceID(joystick1);

	SDL_JoystickEventState(SDL_ENABLE);
	SDL_Event event;

	while(!work_done && SDL_WaitEvent(&event))
	{

		//figure out which one emitted the event
		if (event.type == SDL_JOYAXISMOTION)
			is_p1 = (event.jaxis.which == p1_joy_id);
		else if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP)
			is_p1 = (event.jbutton.which == p1_joy_id);


		pthread_mutex_t* mut = (is_p1) ? &p1_input_lock: &p2_input_lock;
		uint16_t* x_axis_val = (is_p1) ? &p1_x_axis : &p2_x_axis;
		uint16_t* y_axis_val = (is_p1) ? &p1_y_axis : &p2_y_axis;
		uint8_t* input_button_bitmask = (is_p1) ? &p1_input_bitmask : &p2_input_bitmask;

		//modify state
		pthread_mutex_lock(mut);

		switch(event.type){

			//case: axis event -- determine axis and set
			case SDL_JOYAXISMOTION:

				if(event.jaxis.axis == 0)
					axis_val = x_axis_val;
				else
					axis_val = y_axis_val;
					 
				*axis_val = event.jaxis.value;
//				printf("axis val: %d\n", *axis_val);
				break;

			//case; button down -- set bit
			case SDL_JOYBUTTONDOWN:

				input_button_bitmask = (is_p1) ? &p1_input_bitmask : &p2_input_bitmask;
				if(event.jbutton.button == 0)
					*input_button_bitmask |= JOY_BTN_1;
				else if(event.jbutton.button == 1)
					*input_button_bitmask |= JOY_BTN_2;

				
//				printf("button mask: %d\n", *input_button_bitmask);
				break;

			//case; button up -- clear bit
			case SDL_JOYBUTTONUP:

				input_button_bitmask = (is_p1) ? &p1_input_bitmask : &p2_input_bitmask;
				if(event.jbutton.button == 0)
					*input_button_bitmask &= ~JOY_BTN_1;
				else if(event.jbutton.button == 1)
					*input_button_bitmask &= ~JOY_BTN_2;

//				printf("button mask: %d\n", *input_button_bitmask);
				
				break;
			case 256:
				fprintf(stderr, "CTRL-C caught, exiting...\n\n");
				SDL_JoystickClose(joystick1);
				SDL_JoystickClose(joystick2);
				exit(0);

			//idk situation
			default:
				break;
		}

		pthread_mutex_unlock(mut);
	}
	
	SDL_JoystickClose(joystick1);
	SDL_JoystickClose(joystick2);
	
	return NULL;
}
