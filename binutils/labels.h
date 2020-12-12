#ifndef LABELS_H__
#define LABELS_H__

#include <stdint.h>
#include "sections.h"

struct label_entry {
  char name[64];
  uint16_t id;
  uint16_t position;
  uint8_t present;
};


struct label_entry * find_label(char * label, struct label_entry * e);
struct label_entry * find_label_by_id(uint16_t id, struct label_entry * e);
void mark_label_position(char * label, uint16_t pos, struct section * current_section);
uint16_t mark_label_use(char * label, uint16_t addr, struct section * current_section);

uint16_t get_label_usage_list_size();
uint16_t get_label_vec_size();

#endif