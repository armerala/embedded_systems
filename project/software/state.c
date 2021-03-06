#include "state.h"

struct scene_object* player_instantiate(int is_p1)
{
	struct player_state* state;
	struct sprite_data* sd;
	struct scene_object* obj;

	//alloc needed structures
	state = (struct player_state*)malloc(sizeof(struct player_state));
	if(!state)
		return NULL;

	sd = (struct sprite_data*)malloc(sizeof(struct sprite_data));
	if(!sd)
	{
		free(state);
		return NULL;
	}

	obj = (struct scene_object*)malloc(sizeof(struct scene_object));
	if(!obj)
	{
		free(state);
		free(sd);
		return NULL;
	}

	// initialize player state, sprite data
	state->is_p1 = (uint8_t) is_p1;
	state->health = 3;
	sd->magic = IDLE;
	if (is_p1)
		sd->flags = 0;
	else
		sd->flags = SPRITE_FLIP_X;

	obj->update = &player_update;
	obj->die = &player_destroy;
	obj->state = state;
	obj->sd = sd;
	obj->pos = (struct vec2){0, 0};

	return obj;
}

void player_destroy(struct scene_object* player_obj)
{
	free(player_obj->state);
	free(player_obj->sd);
	free(player_obj);
}

void player_update(struct scene_object* player_obj)
{

	int DIFF_TEST = 0;

	struct player_state *state = player_obj->state;

	int b1down = get_button_down(JOY_BTN_1, state->is_p1);
	int b2down = get_button_down(JOY_BTN_2, state->is_p1);
	int b1up = get_button_up(JOY_BTN_1, state->is_p1);
	int b2up = get_button_up(JOY_BTN_2, state->is_p1);
	
	int x_axis = get_axis(1,  state->is_p1);
	int y_axis = get_axis(0, state->is_p1);

	// is player dead?
	if (state->health == 0) {
		player_obj->sd->magic = DEAD;
	}
	
	// buttons
	if (b1down) {
		player_obj->sd->magic = PUNCH;
		DIFF_TEST=1;
	}
	if (b2down) {
		player_obj->sd->magic = KICK;
		DIFF_TEST=1;
	}
	if (b1up || b2up) {
		player_obj->sd->magic = IDLE;
		DIFF_TEST=1;
	}


	// SDL encoding:
	// 32767 is left,down
	// 32768 is right,up


	// axes - TODO:	 FIX THESE MAX/MIN LOCATION VALUES!!

	// TODO: change vector values to floats, not ints. then increment by like 1/10th every time

	if (x_axis == 32768) {
		player_obj->sd->magic = WALK;
		if (player_obj->pos.x != 150) 
			player_obj->pos.x++;

		DIFF_TEST=1;
	}
	if (x_axis == 32767) {
		player_obj->sd->magic = WALK;
		if (player_obj->pos.x != 0)
			player_obj->pos.x--;
		DIFF_TEST=1;
	}
	if (x_axis == 0) 
		player_obj->sd->magic = IDLE;

	if (y_axis == 32768) {
		player_obj->sd->magic = JUMP;
		player_obj->pos.y++;
		DIFF_TEST=1;
	}
	if (y_axis == 32767) { 
		player_obj->sd->magic = DUCK;
		DIFF_TEST=1;
	}
	if (y_axis == 0)
		player_obj->sd->magic = IDLE;


	struct scene_object *other = player_obj->other;

	struct player_state *other_state = other->state;

	int width = 10;

	// TODO: this shouldnt just be x==x, probably some offset/box width
	if (other->pos.x == player_obj->pos.x  ) {
		if (player_obj->sd->magic == KICK || player_obj->sd->magic == PUNCH) {
			other_state->health--;
			printf("took a hit!\n");
		}
		printf("they overlap!\n");
	}

	if (DIFF_TEST) {
		fprintf(stderr, "Player sprite: %d\nPlayer position: %d,%d\nPlayer health; %d\n\n", 
			player_obj->sd->magic, player_obj->pos.x, player_obj->pos.y, state->health);
	}
	

}
