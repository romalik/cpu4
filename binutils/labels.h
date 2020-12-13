#ifndef LABELS_H__
#define LABELS_H__

#include <stdint.h>
#include "sections.h"

struct label_entry {
  char name[64];
  uint16_t id;
  uint16_t position;
  uint8_t present;
  uint8_t export;
  uint8_t import;
};


struct label_entry * find_label(char * label, struct section * sect);
struct label_entry * find_label_by_id(uint16_t id, struct section * sect);
void mark_label_position(char * label, uint16_t pos, struct section * current_section);
uint16_t mark_label_use(char * label, uint16_t addr, struct section * current_section);
void mark_label_export(char * label, struct section * current_section);
void mark_label_import(char * label, struct section * section);

void print_labels(uint8_t * label_vec);
void print_label(struct label_entry * e);


#endif