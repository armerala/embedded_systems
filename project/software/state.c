#include "state.h"

struct scene_object* player_instantiate(int which)
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

	//assign data
	state->which = (uint8_t) which;
	state->health = 3;
	sd->magic = IDLE;

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
	struct player_state *state = player_obj->state;

	get_button_down(state->which);
}
