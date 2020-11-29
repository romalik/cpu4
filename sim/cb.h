#ifndef CB_H__
#define CB_H__

#define DYNAMIC_MEMORY_SUPPORT 1


#if DYNAMIC_MEMORY_SUPPORT
#include <malloc.h>
#endif


struct circular_buffer {
  size_t r;
  size_t w;
  size_t capacity;
  size_t size;
  unsigned int * data;
};

#if DYNAMIC_MEMORY_SUPPORT
struct circular_buffer * cb_create(size_t capacity);
#endif

struct circular_buffer * cb_create_static(size_t capacity, struct circular_buffer * cb, unsigned int * arena);

#if DYNAMIC_MEMORY_SUPPORT
void cb_destroy(struct circular_buffer * cb);
#endif

unsigned int cb_pop(struct circular_buffer * cb);
void cb_push(struct circular_buffer * cb, unsigned int val);
void cb_clear(struct circular_buffer * cb);


#endif
