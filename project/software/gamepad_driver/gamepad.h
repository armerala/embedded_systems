#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__


// some #defines for gamepad state
#define BUTTON_UP 0
#define BUTTON_DOWN 1
#define JOY_CENTER 0
#define JOY_LEFT 1
#define JOY_RIGHT 2
#define JOY_UP 3
#define JOY_DOWN 4


// struct for representing gamepad state
struct gamepad_state
{
	char which; // joystick 0 or 1
	char b0;	// button 0
	char b1;	// button 1
	char x;		// X (L/R) axis of joystick
	char y;		// Y (U/D) axis of joystick
};


// function signatures
void init_gamepads();
void shutdown_gamepads();
struct gamepad_state *get_gamepad_event();


#endif
