#include "sections.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>

void serialize_section(struct section * section, FILE * fp) {
  unsigned char data_size_h;
  unsigned char data_size_l;
  unsigned char label_vec_size_h;
  unsigned char label_vec_size_l;
  unsigned char label_mask_size_h;
  unsigned char label_mask_size_l;
  fwrite(&section->name, SECTION_NAME_SIZE, 1, fp);
  fwrite(&section->executable, 1, 1, fp);
  
  data_size_h = high(section->data_pos);
  data_size_l = low(section->data_pos);
  label_vec_size_h = high(section->label_vec_pos);
  label_vec_size_l = low(section->label_vec_pos);
  label_mask_size_h = high(section->label_mask_pos);
  label_mask_size_l = low(section->label_mask_pos);
  fwrite(&data_size_l, 1, 1, fp);
  fwrite(&data_size_h, 1, 1, fp);
  fwrite(&label_vec_size_l, 1, 1, fp);
  fwrite(&label_vec_size_h, 1, 1, fp);
  fwrite(&label_mask_size_l, 1, 1, fp);
  fwrite(&label_mask_size_h, 1, 1, fp);
  fwrite(&section->data, section->data_pos, 1, fp);
  fwrite(&section->label_vec, section->label_vec_pos, 1, fp);
  fwrite(&section->label_mask, section->label_mask_pos, 1, fp);
}

struct section * deserialize_section(FILE * fp) {
  struct section * section;
  unsigned char data_size_h;
  unsigned char data_size_l;
  unsigned char label_vec_size_h;
  unsigned char label_vec_size_l;
  unsigned char label_mask_size_h;
  unsigned char label_mask_size_l;

  section = (struct section *)malloc(sizeof(struct section));

  fread(&section->name, SECTION_NAME_SIZE, 1, fp);
  fread(&section->executable, 1, 1, fp);

  fread(&data_size_l, 1, 1, fp);
  fread(&data_size_h, 1, 1, fp);

  section->data_pos = (data_size_h<<8) | (data_size_l);

  fread(&label_vec_size_l, 1, 1, fp);
  fread(&label_vec_size_h, 1, 1, fp);

  section->label_vec_pos = (label_vec_size_h<<8) | (label_vec_size_l);

  fread(&label_mask_size_l, 1, 1, fp);
  fread(&label_mask_size_h, 1, 1, fp);

  section->label_mask_pos = (label_mask_size_h<<8) | (label_mask_size_l);

  fread(&section->data, section->data_pos, 1, fp);
  fread(&section->label_vec, section->label_vec_pos, 1, fp);
  fread(&section->label_mask, section->label_mask_pos, 1, fp);

  //printf("name: %s executable: %d data_pos %d label_vec_pos %d label_mask_pos %d\n", section->name, section->executable, section->data_pos, section->label_vec_pos, section->label_mask_pos);

  return section;
}
