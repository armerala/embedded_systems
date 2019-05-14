#include "physics.h"


void update_physics() {
	iter_scene(&__update_physics);
}

static struct scene_object* __current_obj;
static void __update_physics(struct scene_object* obj)
{
	if(obj->collider != NULL) {
		__current_obj = obj;
		//__update_gravity(obj);
		iter_scene(&__update_physics_internal);
	}
}

static void __update_physics_internal(struct scene_object* other)
{
	__resolve_collision(__current_obj, other);
}

//static void __apply_gravity(struct


/**
 *  tries to resolve collision between two objects
 *  If one or the other has no physics associated with it, then 
 *  the function does nothing.
 */
static void __resolve_collision(struct scene_object* obj1, struct scene_object* obj2)
{
	if(obj1->collider == NULL || obj2->collider == NULL)
		return;

	//check if overlap. Positive means overlap, negative means not
	struct vec2 overlap;

	if(bl1.x < bl2.x)
		overlap.x = tr1.x - bl2.x;
	else
		overlap.x = tr2.x - bl1.x;

	if(bl1.y < bl2.y)
		overlap.y = tr1.y - bl2.y;
	else
		overlap.y = tr2.y - bl1.y

	//split the distance and push the two apart
	if(overlap.x < 0 || overlap.y < 0)
		return;

	obj1->pos.x += overlap.x / 2;
	obj1->pos.y += overlap.y / 2;

	obj2->pos.x += overlap.x / 2;
	obj2->pos.y += overlap.y / 2;
}
