#ifndef __INPUT_H__
#define __INPUT_H__ 

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_joystick.h>
#include <SDL_events.h>

#define JOY_BTN_1 (1 << 0)
#define JOY_BTN_2 (1 << 1)

extern int init_input();
extern void shutdown_input();
extern void update_inputs();
extern int get_button_down(int keycode, int is_p1);
extern int get_button_up(int keycode, int is_p1);
extern int get_button(int keycode, int is_p1);

extern void* __handle_joy(void* arg);


#endif
