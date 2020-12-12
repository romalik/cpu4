#ifndef SECTIONS_H__
#define SECTIONS_H__


#define MAX_SECTIONS 15
#define MAX_SECTION_SIZE 65535
#define MAX_LABEL_VEC_SIZE 65535*16
#define MAX_LABEL_MASK_SIZE 65535*16


struct section {
  char name[32];
  char data[MAX_SECTION_SIZE];
  char label_vec[MAX_LABEL_VEC_SIZE];
  char label_mask[MAX_LABEL_MASK_SIZE];
  int data_pos;
  struct label_entry * label_vec_pos;
  uint16_t * label_mask_pos;
};

#endif