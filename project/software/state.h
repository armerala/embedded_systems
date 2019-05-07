#ifndef __STATE_H__
#define __STATE_H__

#include <stdint.h>

#include "render.h"
#include "scene.h"

struct player_state
{
	uint8_t health; // 0-3
};

extern struct scene_object* player_instantiate();
extern void player_destroy(struct scene_object* player_obj);
extern void player_update(struct scene_object* player_obj);



#endif
