#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sections.h"
#include "util.h"
#include "labels.h"
#include <assert.h>


static uint16_t current_label_id = 0;


void print_label(struct label_entry * e) {
    printf("0x%04x\t0x%04x\t%d\t%d\t%s\n", e->id, e->position, e->present, e->export, e->name);
}

void print_labels(uint8_t * label_vec) {
  struct label_entry * e = (struct label_entry *)label_vec;

  printf("Labels:\nID\tpos\tpres\texport\tname\n");

  while(*(e->name)) {
    print_label(e);
    e++;
  }

}


static struct label_entry * create_new_label(char * label, struct section * current_section) {
  struct label_entry * new_label = (struct label_entry *)(&current_section->label_vec[current_section->label_vec_pos]);
  strcpy(new_label->name, label);
  new_label->id = current_label_id;
  new_label->position = 0;
  new_label->present = 0;
  new_label->export = 0;
  new_label->import = 0;
  
  current_label_id++;
  current_section->label_vec_pos+=sizeof(struct label_entry);
  return new_label;
}


struct label_entry * find_label(char * label, struct section * sect) {
  int i;
  struct label_entry * e;
  for(i = 0; i<sect->label_vec_pos; i+=sizeof(struct label_entry)) {
    e = (struct label_entry *)(&sect->label_vec[i]);
    if(!strcmp(e->name, label)) {
      return e;
    }
  }
  return 0;
}

struct label_entry * find_label_by_id(uint16_t id, struct section * sect) {
  int i;
  struct label_entry * e;
  for(i = 0; i<sect->label_vec_pos; i+=sizeof(struct label_entry)) {
    e = (struct label_entry *)(&sect->label_vec[i]);
    if(e->id == id) {
      return e;
    }
  }
  return 0;
}

void mark_label_position(char * label, uint16_t pos, struct section * current_section) {
  struct label_entry * e;

  e = find_label(label, current_section);
  if(!e) {
    e = create_new_label(label, current_section);
  }

  e->position = pos;
  e->present = 1;
}

void mark_label_export(char * label, struct section * current_section) {
  struct label_entry * e;

  e = find_label(label, current_section);
  if(!e) {
    fprintf(stderr, "Try export unregistered label\n");
    assert(0);
  }

  e->export = 1;
}

void mark_label_import(char * label, struct section * section) {
  struct label_entry * e;

  e = find_label(label, section);
  if(e) {
    e->import = 1;
  }
}

int calculate_offset_sum(char * str) {
  char * s = str;
  char * p;
  int offset = 0;
  int sign = 1;
  while(1) {
    p = strchr(s, '+');
    if(p == NULL) {
      p = strchr(s, '-');
    }
    if(p == NULL) return offset;

    s = p;
    if(*s == '+') {
      sign = 1;
    } else if(*s == '-') {
      sign = -1;
    }
    s++;
    offset += sign * atoi(s);
  }
}

uint16_t mark_label_use(char * label, uint16_t addr, struct section * current_section) {
  struct label_entry * e;
  uint16_t * mask_ptr;
  int offset = 0;
  char * sign_pos = NULL;

  sign_pos = strchr(label, '+');
  if(!sign_pos) {
    sign_pos = strchr(label, '-');
  }

  if(sign_pos) {
    offset = calculate_offset_sum(sign_pos);
//    fprintf(stderr, "asm: label %s offset %d\n", label, offset);
    *sign_pos = 0;
  }

  e = find_label(label, current_section);
  if(!e) {
    e = create_new_label(label, current_section);
  }

  mask_ptr = (uint16_t *)(&current_section->label_mask[current_section->label_mask_pos]);

  *mask_ptr = addr;

  mask_ptr++;  
  *mask_ptr = offset;

  current_section->label_mask_pos += 4;

  return e->id;
}
