#ifndef ROBJ_H__
#define ROBJ_H__

#include <stdint.h>

#define ROBJ_TYPE_EXEC    0
#define ROBJ_TYPE_OBJECT  1


#define ROBJ_SIGNATURE "robj"


struct robj_header {
  char signature[4];
  char type;
  int n_sections;
};



#endif