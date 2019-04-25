#ifndef __SCENE_H__
#define __SCENE_H__

#include <stdlib.h>

#include "state.h"

struct scene_object
{
    void (*update)(void*, struct sprite_data*); //the update procedure (takes in state and sprite data)
    void* state;                                //the custom state data
    struct sprite_data* sd;                     //sprite data (may be NULL if not rendered)
};

struct scene_node
{
    struct scene_node* next;
    struct scene_node* prev;
    struct scene_object* data;
};

typedef struct scene_node* scene_handle; 
#define NULL_HANDLE NULL;

//init/shutdown/update
extern int init_scene();

extern void update_scene();
extern void __do_update(struct scene_object* obj);

//scene add/remove
extern scene_handle scene_add(struct scene_object* obj);
extern void scene_remove(scene_handle handle);

//util
extern void iter_scene( void(*func)(struct scene_object*) );

#endif
