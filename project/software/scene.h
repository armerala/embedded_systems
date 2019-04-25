#ifndef __SCENE_H__
#define __SCENE_H__

#include "state.h"

/**
 * A public interface for creating and managing game objects
 */
struct scene_object
{
    void (*update)(void*, struct sprite_data*); //the update procedure (takes in state and sprite data)
    void* state;                                //the custom state data
    struct sprite_data* sd;                     //sprite data (may be NULL if not rendered)
};

/**
 * a linked list node because C doesn't provide one
 *
 * NOTE: linked list is fine because our scene is small. This is simple
 * to implement, and the probably the least of our performance worries
 */
struct scene_node
{
    scene_node* next;
    scene_node* prev;
    scene_object* data;
};

typedef scene_node* scene_handle; 
#define NULL_HANDLE NULL;

extern int init_scene();

extern void update_scene();

extern scene_handle scene_add(scene_object* obj);
extern void scene_remove(scene_handle handle);

extern void iter_scene( void(*func)(scene_object*) );

#endif
