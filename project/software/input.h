#ifndef __INPUT_H__
#define __INPUT_H__ 

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>


extern int init_input();
extern void shutdown_input();
extern void update_inputs();
extern int get_button_down(int keycode, int is_p1);
extern int get_button_up(int keycode, int is_p1);
extern int get_button(int keycode, int is_p1);

extern void* __handle_input(void* arg);


#endif
