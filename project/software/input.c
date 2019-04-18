#include "input.h"

static pthread_t thread1;
static pthread_mutex_t p1_input_lock;

static pthread_t thread2;
static pthread_mutex_t p2_input_lock;

static uint32_t p1_input_bitmask;
static uint32_t p1_input_bitmask_prev;
static uint32_t p1_input_bitmask_down;
static uint32_t p1_input_bitmask_up;

static uint32_t p2_input_bitmask;
static uint32_t p2_input_bitmask_prev;
static uint32_t p2_input_bitmask_down;
static uint32_t p2_input_bitmask_up;

static void* handle_input(void* arg);

/**
 * return 0 on success, anything else is failure
 */
int init_input() 
{
	if (pthread_mutex_init(&p2_input_lock, NULL) != 0){
	    fprintf(stderr, "failed to initialize mutex\n");
	    return 1;
	}
	if (pthread_mutex_init(&p1_input_lock, NULL) != 0){
	    fprintf(stderr, "failed to initialize mutex\n");
	    return 1;
    }

    int ret1, ret2;

	ret1 = pthread_create(&thread1, NULL, handle_input, NULL);
	ret2 = pthread_create(&thread2, NULL, handle_input, NULL);

    return (ret1==0 && ret2==0);
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

    p2_input_bitmask_down = p2_input_bitmask & ~p2_input_bitmask_prev;
    p2_input_bitmask_up = ~p2_input_bitmask & p2_input_bitmask_prev;
    p2_input_bitmask_prev = p2_input_bitmask;

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

/**
 * A worker for handling input in the background
 */
static void* handle_input(void* arg) 
{
	for (;;)
	{
		// TODO: blocking joystick input call

		// based on what it returned, update arg->state, then set arg->set

	}
    return NULL;
}
