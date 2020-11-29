#include "cb.h"


#if DYNAMIC_MEMORY_SUPPORT
struct circular_buffer * cb_create(size_t capacity) {
  struct circular_buffer * cb;
  unsigned int * arena;
  cb = (struct circular_buffer *)(malloc(sizeof(struct circular_buffer)));
  arena = (unsigned int *)(malloc(capacity));
  return cb_create_static(capacity,cb,arena);
}
#endif

struct circular_buffer * cb_create_static(size_t capacity, struct circular_buffer * cb, unsigned int * arena) {
  cb->data = arena;
  cb->capacity = capacity;
  cb->r = 0;
  cb->w = 0;
  cb->size = 0;
  return cb;
}

#if DYNAMIC_MEMORY_SUPPORT
void cb_destroy(struct circular_buffer * cb) {
  free(cb->data);
  free(cb);
}
#endif

unsigned int cb_pop(struct circular_buffer * cb) {
  unsigned int rval = 0;
  if(cb->size) {
    rval = cb->data[cb->r];
    cb->r++;
    if(cb->r >= cb->capacity) cb->r = 0;
    cb->size--;
  }
  return rval;
}

void cb_push(struct circular_buffer * cb, unsigned int val) {
  cb->data[cb->w] = val;
  cb->w++;
  if(cb->w >= cb->capacity) cb->w = 0;
  if(cb->size < cb->capacity) {
    cb->size++;
  } else {
    cb->r = cb->w;
  }
}

void cb_clear(struct circular_buffer * cb) {
  cb->size = 0;
  cb->w = 0;
  cb->r = 0;
}
