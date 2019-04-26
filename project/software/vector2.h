#ifndef __VECTOR_2_H__
#define __VECTOR_2_H__

#include <stdint.h>

struct vec2
{
	int16_t x;
	int16_t y;
};

inline struct vec2 vec2_add(struct vec2 a, struct vec2 b)
{
	return (struct vec2){a.x + b.x, a.y + b.y};
}

inline struct vec2 vec2_sub(struct vec2 a, struct vec2 b)
{
	return (struct vec2){a.x - b.x, a.y - b.y};
}

#endif
