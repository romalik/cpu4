#include "link.h"
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "labels.h"
#include <stdint.h>
#include <stdlib.h>
#include "robj.h"
#include "sections.h"
#include <assert.h>
#include <string.h>

#define MAXFILES 1000


struct object_file {
  char * name;
  struct robj_header header;
  struct section ** sections;
};

struct object_file * object_files[MAXFILES];

uint8_t loaded_files = 0;


void linker_init() {

}





void load_file(char * path) {
  FILE *f = fopen(path, "rb");
  int i;
  struct object_file * obj;

  obj = object_files[loaded_files] = (struct object_file *)malloc(sizeof(struct object_file));


  fread(&obj->header, sizeof(struct robj_header), 1, f);

  if(memcmp(obj->header.signature, ROBJ_SIGNATURE, 4)) {
    fprintf(stderr, "File %s not an ROBJ\n", path);
    exit(1);
  }

  if(obj->header.type != ROBJ_TYPE_OBJECT) {
    fprintf(stderr, "File %s type not object\n", path);
    exit(1);
  }

  obj->sections = (struct section **)malloc(obj->header.n_sections * sizeof(struct section *));

  obj->name = strdup(path);

  for(i = 0; i<obj->header.n_sections; i++) {
    obj->sections[i] = deserialize_section(f);
  }
  loaded_files++;

  fclose(f);
}

struct section_list_entry {
  struct object_file * origin;
  struct section * section;
  struct section_list_entry * next;
  int used;
  uint16_t start;
};

struct section_dict {
  char name[32];
  char executable;
  struct section_list_entry * list;
};

#define MAX_DICT_SIZE 255

struct section_dict * sects[MAX_DICT_SIZE];
int sects_sz = 0;

static struct section_dict * get_dict_by_name(char * name) {
  int i;
  for(i = 0; i<sects_sz; i++) {
    if(!strcmp(sects[i]->name, name)) {
      return sects[i];
    }
  }
  sects[sects_sz] = (struct section_dict *)malloc(sizeof(struct section_dict));
  strcpy(sects[sects_sz]->name, name);
  sects[sects_sz]->list = NULL;
  sects[sects_sz]->executable = -1;
  sects_sz++;
  return sects[sects_sz - 1];
}

void append_to_sect_list(struct section_dict * dict, struct section * section, struct object_file * origin) {
  if((dict->executable != section->executable) && (dict->executable != -1)) {
    fprintf(stderr, "Both executable and non-executable sections with same name\n");
    assert(0);
  }

  dict->executable = section->executable;

  if(dict->list == NULL) {
    dict->list = (struct section_list_entry *)malloc(sizeof(struct section_list_entry));
    dict->list->next = NULL;
    dict->list->section = section;
    dict->list->origin = origin;
    dict->list->used = 0;
    dict->list->start = 0;
  } else {
    struct section_list_entry * node;
    node = dict->list;
    while(node->next != NULL) {
      node = node->next;
    }
    node->next = (struct section_list_entry *)malloc(sizeof(struct section_list_entry));
    node->next->next = NULL;
    node->next->section = section;
    node->next->origin = origin;
    node->next->used = 0;
    node->next->start = 0;
  }
}

void build_section_lists() {
  int i,j;
  int sects_sz = 0;
  struct section_dict * sd;
  memset(sects, 0, MAX_DICT_SIZE*sizeof(struct section_dict *));

  for(i = 0; i<loaded_files; i++) {
    for(j = 0; j<object_files[i]->header.n_sections; j++) {
      char * sect_name = object_files[i]->sections[j]->name;
      sd = get_dict_by_name(sect_name);
      append_to_sect_list(sd, object_files[i]->sections[j], object_files[i]);
    }
  }  

}


void linker_offset_labels() {
  int i, j;
  uint16_t c_off = 0;
  struct label_entry * e;

  for(i = 0; i < sects_sz; i++) {
    struct section_list_entry * node;
    node = sects[i]->list;
    while(node) {
      if(node->used) {
        node->start = c_off;

        e = (struct label_entry *)node->section->label_vec;
        for(j = 0; j<node->section->label_vec_pos / sizeof(struct label_entry); j++) {
          if(e[j].present) e[j].position += c_off;
        }

        c_off += node->section->data_pos;
      }
      node = node->next;
    }    
  }
}





struct label_entry * find_located_label(uint16_t id, struct section * sect, struct object_file* origin) {
  int i,j;
  char * name;
  struct label_entry * e = NULL;

  //first find label name and return if it is present

  e = find_label_by_id(id, sect);

  if(e->present) {
    //label exists in the same section
    return e;
  }

  if(!e) {
    fprintf(stderr, "Corrupt label vec");
    exit(1);
  }

  //now try to find it in same file
  name = e->name;

  for(i = 0; i<origin->header.n_sections; i++) {
    struct label_entry * e_;
    e_ = find_label(name, origin->sections[i]);
    if(e_) {
      e = e_;
    }
    if(e->present) {
      //label exists in the same file
      return e;
    }
  }

  if(!e) {
    fprintf(stderr, "Corrupt label vec");
    exit(1);
  }

  //search across other label vecs
  for(i = 0; i<loaded_files; i++) {
    for(j = 0; j<object_files[i]->header.n_sections; j++) {
      e = find_label(name, object_files[i]->sections[j]);
      if(e) {
        if(e->present && e->export) {
          return e;
        }
      }
    }
  }

  printf("Symbol %s not found!\n", name);
  exit(1);

  return 0;
}

void linker_link() {
  struct label_entry * e;
  int curr_file;
  int i, j;
  int offset;
  uint16_t addr;
  uint16_t id;


  for(i = 0; i < sects_sz; i++) {
    struct section_list_entry * node;
    node = sects[i]->list;
    while(node) {
      if(node->used) {
        for(j = 0; j<node->section->label_mask_pos; j+=4) {
          addr = *(uint16_t *)(&node->section->label_mask[j]);
          offset = *(uint16_t *)(&node->section->label_mask[j+2]);
          id = ((uint8_t)node->section->data[addr]) | ((uint8_t)node->section->data[addr+1] << 8);

          e = find_located_label(id, node->section, node->origin);
          if(e) {
            node->section->data[addr] = low((e->position + offset));
            node->section->data[addr+1] = high((e->position + offset));
          } else {
            exit(1);
          }
        }
      }
      node = node->next;
    }    
  }
}

int sect_comparator(const void * _a, const void * _b) {
  const struct section_dict * a = *(const struct section_dict **)_a;
  const struct section_dict * b = *(const struct section_dict **)_b;

  //place .section start_section first
  if(!strcmp(a->name, "start_section")) {
    return -1;
  }

  if(!strcmp(b->name, "start_section")) {
    return 1;
  }


  //place executable sections first
  if(a->executable && !b->executable) {
    return -1;
  }
  if(!a->executable && b->executable) {
    return 1;
  }


  return strcmp(a->name, b->name);

}


void sort_sects() {
  //sort sections here (all text sections go before all data sections)

  qsort(sects, sects_sz, sizeof(struct section_dict *), sect_comparator);

}

struct section_list_entry * find_section_in_dict(struct section * ptr) {
  int i;
  for(i = 0; i < sects_sz; i++) {
    struct section_list_entry * node;
    node = sects[i]->list;
    while(node) {
      if(node->section == ptr) {
        return node;
      }
      node = node->next;
    }
  }
  return NULL;
}

//forward
void require_section(struct section_list_entry * node);

void require_label(char * label_name, struct section_list_entry * from_node) {
  int i,j;

  //search label in file first
  if(from_node) {
    for(i = 0; i<from_node->origin->header.n_sections; i++) {
      struct label_entry * e = NULL;
      e = find_label(label_name, from_node->origin->sections[i]);
      if(e) {
        if(e->present) {
          struct section_list_entry * node = find_section_in_dict(from_node->origin->sections[i]);
          if(!node) {
            fprintf(stderr, "Section not found!\n");
            exit(1);
          }
          if(node->used == 0) {
//            printf("Including node %s from %s by label '%s'\n", node->section->name, node->origin->name, label_name);
            node->used = 1;
            require_section(node);
          }
          return;
        }
      }
    }
  }

  //search for exports now in other files
  for(i = 0; i < sects_sz; i++) {
    struct section_list_entry * node;
    node = sects[i]->list;
    while(node) {
      struct label_entry * e = NULL;
      e = find_label(label_name, node->section);
      if(e) {
        if(e->present && e->export) {
          if(node->used == 0) {
//            printf("Including node %s from %s by label '%s'\n", node->section->name, node->origin->name, label_name);
            node->used = 1;
            require_section(node);
          }
          return;
        }
      }
      node = node->next;
    }
  }

  fprintf(stderr, "No one exports label '%s'\n", label_name);
  exit(1);
}

void require_section(struct section_list_entry * node) {
  int i;
  struct label_entry * e;
  for(i = 0; i<node->section->label_vec_pos; i+=sizeof(struct label_entry)) {
    e = (struct label_entry *)(&node->section->label_vec[i]);
    if(!e->present) {
      require_label(e->name, node);
    } 
  }
}

int fl_remove_unused_sections = 1;

void mark_used_sects() {
  int i;
  if(fl_remove_unused_sections) {
    require_label("__entry", NULL);
  } else {
    for(i = 0; i < sects_sz; i++) {
      struct section_list_entry * node;
      node = sects[i]->list;
      while(node) {
        node->used = 1;
        node = node->next;
      }
    }
  }

}



char * output_fname;

char * raw_data[MAXFILES];

int main(int argc, char ** argv) {
  FILE *f_out;
  int i = 0;
  int j = 0;
  int file_ct = 0;
  size_t total_size = 0;
  uint16_t obj_size;
  uint16_t label_vec_size;
  uint16_t label_mask_size;

  output_fname = "out.bin";

  for(i = 1; i<argc; i++) {
    if(*argv[i] == '-') {
      if(!strcmp(&argv[i][1], "o")) {
        output_fname = argv[i+1];
      }
      i++;
    } else {
      load_file(argv[i]);
    }
  }

  build_section_lists();

  mark_used_sects();
  /*
  printf("Loaded labels:\n");
  for(i = 0; i<loaded_files; i++) {
    printf("file %d:\n", i);
    print_labels(label_vecs[i]);
  }
  */

  sort_sects();

  linker_offset_labels();

/*
  printf("off:\n");
  for(i = 0; i<loaded_files; i++) {
    printf("file %d:\n", i);
    print_labels(label_vecs[i]);
  }
*/
/*
  for(i = 0; i < sects_sz; i++) {
    struct section_list_entry * node;
    printf("sect %s in\n", sects[i]->name);
    node = sects[i]->list;
    while(node) {
      printf("\t%s from %s\n", node->section->name, node->origin->name);
      node = node->next;
    }    
  }
*/
  linker_link();



  f_out = fopen(output_fname, "wb");

  for(i = 0; i < sects_sz; i++) {
    struct section_list_entry * node;
    node = sects[i]->list;
    while(node) {
      if(node->used) {
        total_size += fwrite(node->section->data, 1, node->section->data_pos, f_out);
      }
      printf("%d\t0x%04x\t(%c) <%s> [%s] %s\n", node->section->data_pos, node->start,  node->used?'+':' ', node->section->executable?"text":"data", node->section->name, node->origin->name);
      node = node->next;
    }    
  }
  fclose(f_out);

  printf("size %s: %d\n", output_fname, total_size);
  return 0;

}