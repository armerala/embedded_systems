#include "scene.h"

static struct scene_node scene_list = {&scene_list, &scene_list, NULL};

/**
 * Initializes the scene
 *
 * @returns 0 on success, 1 otherwise
 */
int init_scene()
{
    struct scene_object* ps1 = player_instantiate(1);
    struct scene_object* ps2 = player_instantiate(0);
	ps1->pos = (struct vec2){4, 10};
	ps2->pos = (struct vec2){10, 10};

	if(!ps1 || !ps2)
		return -1;

	ps1->other = ps2;
	ps2->other = ps1;
	scene_add(ps1);
	scene_add(ps2);

	return 0;
}

/**
 * shuts down the scene by freeing scene list
 */
void shutdown_scene()
{
    struct scene_node* node = &scene_list;
    while( (node = scene_list.next) != &scene_list )
	{
		struct scene_object* obj = (struct scene_object*)node->data;
		obj->die(obj);
        scene_remove(node);
	}
}

/**
* Iters the scene and calls update handler on all objects
*/
void update_scene()
{
    iter_scene(&__do_update_scene);
}

void __do_update_scene(struct scene_object* obj)
{
    if (obj->update != NULL)
		obj->update(obj);
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
	__scene_remove(node);
}

void __scene_remove(struct scene_node* node)
{
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
