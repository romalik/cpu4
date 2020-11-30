#ifndef ROBJ_H__
#define ROBJ_H__

#include <stdint.h>

struct robj_header {
  char signature[4];
  uint8_t obj_size_l; uint8_t obj_size_h;
  uint8_t label_vec_size_l; uint8_t label_vec_size_h;
  uint8_t label_mask_size_l; uint8_t label_mask_size_h;
};


#endif