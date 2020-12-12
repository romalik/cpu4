#ifndef ROBJ_H__
#define ROBJ_H__

#include <stdint.h>

#define ROBJ_TYPE_EXEC 0
#define ROBJ_TYPE_LIB  1





struct robj_header {
  char signature[4];
  char type;
  char n_sections;
};

struct robj_section_header {
  char name[32];
  uint8_t obj_size_l; uint8_t obj_size_h;
  union {
    struct {
      uint8_t label_vec_size_l; uint8_t label_vec_size_h;
      uint8_t label_mask_size_l; uint8_t label_mask_size_h;
    } labels;
    struct {
      uint8_t load_at_l; uint8_t load_at_h;
      uint8_t start_at_l; uint8_t start_at_h;
    } load_info;
  };
};


#endif