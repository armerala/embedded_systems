#include "scene.h"

struct player_state ps1, ps2;
static struct scene_node scene_list = { &scene_list, &scene_list, NULL };

/**
 * Initializes the scene
 */
int init_scene()
{
    ps1 = (struct player_state){ 3, (struct sprite_data){4, 10, IDLE} };
    ps2 = (struct player_state){ 3, (struct sprite_data){10, 10, IDLE} };
	
	return 0;
}

/**
* Iters the scene and calls update handler on all objects
*/
void update_scene()
{
    iter_scene(&__do_update);
}

void __do_update(struct scene_object* obj)
{
    obj->update(obj->state, obj->sd);
}

/**
 * Add a scene node in using the scene_object as data
 * 
 * @param obj: the scene_object to add to the scene
 */
scene_handle scene_add(struct scene_object* obj)
{
	//alloc new node
    struct scene_node* new_node = (struct scene_node*)malloc(sizeof(struct scene_node));
    if(new_node == NULL)
        return NULL_HANDLE;

    new_node->data = obj;

	//patch pointers
    new_node->next = scene_list.next;
    new_node->prev = &scene_list;

    scene_list.next->prev = new_node;
    scene_list.next = new_node;

    return (scene_handle)new_node;
}

/**
 * removes a node from the scene
 */
void scene_remove(scene_handle handle)
{
    struct scene_node* node = (struct scene_node*)handle;

    node->next->prev = node->prev;
    node->prev->next = node->next;
    free(node);
}

/**
 * Allow a function to iter the whole scene
 * and run a function on it (e.g. for rendering)
 */
void iter_scene( void (*func)(struct scene_object*) )
{
    struct scene_node* node = &scene_list;

    while( (node = node->next) != &scene_list )
        func(node->data);
}
