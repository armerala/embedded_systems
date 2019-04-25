#include "scene.h"

struct player_state ps1, ps2;
static scene_node scene_list;

int init_scene()
{
    scene_list = {&scene_list, &scene_list, NULL};
    ps1 = { {4, 10, IDLE}, 3 };
    ps2 = { {10, 10, IDLE}, 3 };
	
	return 0;
}


void update_scene()
{
    iter_scene(&do_update);
}

static void do_update(scene_object* obj)
{
    obj->update(obj->state, obj->sd);
}

/**
 * Add a scene node in using the scene_object as data
 */
void scene_handle scene_add(scene_object* obj)
{
    scene_node* new_node = (scene_object*)malloc(sizeof(scene_node));
    if(new_node == NULL)
        return NULL_HANDLE;

    new_node->data = obj;
    scene_add(&scene_list, new_node);
    return (scene_handle)new_node;
}

static void scene_add(scene_node* prev, scene_node* new_node);
{
    new_node->next = prev->next;
    new_node->prev = prev;

    prev->next->prev = new_node;
    prev->next = new_node;
}

/**
 * removes a node from the scene
 */
void scene_remove(scene_handle handle)
{
    scene_node* node = (scene_node*)handle;

    node->next->prev = node->prev;
    node->prev->next = node->next;
    free(node);
}

/**
 * Allow a function to iter the whole scene
 * and run a function on it (e.g. for rendering)
 */
void iter_scene( void (*func)(scene_object*) )
{
    scene_node* node = &scene_list;

    while( (node = node->next) != &scene_list )
        func(node->data);
}
