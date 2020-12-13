#ifndef SECTIONS_H__
#define SECTIONS_H__

#include <stdio.h>

#define MAX_SECTIONS 2048
#define MAX_SECTION_SIZE 65535
#define MAX_LABEL_VEC_SIZE 65535*16
#define MAX_LABEL_MASK_SIZE 65535*16
#define SECTION_NAME_SIZE 32

struct section {
  char name[SECTION_NAME_SIZE];
  char executable;
  char data[MAX_SECTION_SIZE];
  char label_vec[MAX_LABEL_VEC_SIZE];
  char label_mask[MAX_LABEL_MASK_SIZE];
  int data_pos;
  int label_vec_pos;
  int label_mask_pos;  
};

void serialize_section(struct section * section, FILE * fp);
struct section * deserialize_section(FILE * fp);

#endif