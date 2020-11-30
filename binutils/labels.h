#ifndef LABELS_H__
#define LABELS_H__

#include <stdint.h>


struct label_entry {
  char name[64];
  uint16_t id;
  uint16_t position;
  uint8_t present;
};


void set_label_vec(uint8_t * vec);
void set_label_usage_list(char * vec);
struct label_entry * find_label(char * label, struct label_entry * e);
struct label_entry * find_label_by_id(uint16_t id, struct label_entry * e);
void mark_label_position(char * label, uint16_t pos);
uint16_t mark_label_use(char * label, uint16_t addr);

uint16_t get_label_usage_list_size();
uint16_t get_label_vec_size();

#endif