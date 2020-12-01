#include <stdint.h>
#include <string.h>
#include "util.h"
#include "labels.h"

static uint16_t current_label_id = 0;
static struct label_entry * current_label_position;
static struct label_entry * label_vec;

static uint16_t * label_usage_list;
static uint16_t * label_usage_list_pos;


void set_label_usage_list(char * vec) {
  label_usage_list = (uint16_t *)vec;
  label_usage_list_pos = (uint16_t *)vec;
}

void set_label_vec(uint8_t * vec) {
  current_label_position = (struct label_entry *)vec;
  label_vec = (struct label_entry *)vec;
}

uint16_t get_label_usage_list_size() {
  return (char *)label_usage_list_pos - (char *)label_usage_list;
}

uint16_t get_label_vec_size() {
  return (char *)current_label_position - (char *)label_vec;
}




static struct label_entry * create_new_label(char * label) {
  struct label_entry * new_label = current_label_position;
  strcpy(new_label->name, label);
  new_label->id = current_label_id;

  current_label_id++;
  current_label_position++;
  return new_label;
}


struct label_entry * find_label(char * label, struct label_entry * e) {
  while(*(e->name)) {
    if(!strcmp(e->name, label)) {
      return e;
    }
    e++;
  }
  return 0;
}

struct label_entry * find_label_by_id(uint16_t id, struct label_entry * e) {
  while(*(e->name)) {
    if(e->id == id) {
      return e;
    }
    e++;
  }
  return 0;
}

void mark_label_position(char * label, uint16_t pos) {
  struct label_entry * e;

  e = find_label(label, label_vec);
  if(!e) {
    e = create_new_label(label);
  }

  e->position = pos;
  e->present = 1;
}


uint16_t mark_label_use(char * label, uint16_t addr) {
  struct label_entry * e;
  int offset = 0;
  char * sign_pos = NULL;

  sign_pos = strchr(label, '+');
  if(!sign_pos) {
    sign_pos = strchr(label, '-');
  }

  if(sign_pos) {
    offset = atoi(sign_pos+1);
    if(*sign_pos == '-') {
      offset = -offset;
    }
    *sign_pos = 0;
  }

  e = find_label(label, label_vec);
  if(!e) {
    e = create_new_label(label);
  }

  *label_usage_list_pos = addr;
  label_usage_list_pos++;
  /*
  *label_usage_list_pos = addr;
  label_usage_list_pos++;
  */
  return e->id;
}
