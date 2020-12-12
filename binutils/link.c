#include "link.h"
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "labels.h"
#include <stdint.h>
#include <stdlib.h>
#include "robj.h"
#include "sections.h"

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

  printf("load file %s\n", path);
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

  for(i = 0; i<obj->header.n_sections; i++) {
    obj->sections[i] = deserialize_section(f);
    printf("deserialize %s\n", obj->sections[i]->name);
  }
  loaded_files++;

  fclose(f);
}

struct section_list_entry {
  struct object_file * origin;
  struct section * section;
  struct section_list_entry * next;
};

struct section_dict {
  char name[32];
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
  sects_sz++;
  return sects[sects_sz - 1];
}

void append_to_sect_list(struct section_dict * dict, struct section * section, struct object_file * origin) {
  if(dict->list == NULL) {
    dict->list = (struct section_list_entry *)malloc(sizeof(struct section_list_entry));
    dict->list->next = NULL;
    dict->list->section = section;
    dict->list->origin = origin;
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
      printf("reg sect name %s\n", sect_name);
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

      e = (struct label_entry *)node->section->label_vec;
      for(j = 0; j<node->section->label_vec_pos; j++) {
        if(e[j].present) e[j].position += c_off;
      }

      c_off += node->section->data_pos;
      node = node->next;
    }    
  }
}





struct label_entry * find_located_label(uint16_t id, struct object_file* origin) {
  int i;
  char * name;
  struct label_entry * e;
  for(i = 0; i<origin->header.n_sections; i++) {
    e = find_label_by_id(id, (struct label_entry *)origin->sections[i]);
    if(e) break;
  }
  if(!e) {
    printf("Corrupt label vec!\n");
    exit(1);
  }

  if(e->present) {
    //label exists in the same file
    return e;
  }

  //search across other label vecs
  name = e->name;
  for(i = 0; i<loaded_files; i++) {
    if(i == file) continue;
    for(i = 0; i<origin->header.n_sections; i++) {
      e = find_label_by_id(id, (struct label_entry *)origin->sections[i]);
    }

//    e = find_label(name, (struct label_entry *)label_vecs[i]);
    if(e) {
      if(e->present) {
        return e;
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
      for(j = 0; j<node->section->label_mask_pos; j+=4) {
        addr = *(uint16_t *)(&node->section->label_mask[j]);
        offset = *(uint16_t *)(&node->section->label_mask[j+2]);
        id = (node->section->data[addr]) | (node->section->data[addr+1] << 8);

        e = find_located_label(id, curr_file);
        if(e) {
          node->section->data[addr] = low((e->position + offset));
          node->section->data[addr+1] = high((e->position + offset));
        } else {
          exit(1);
        }
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
  int file_cnt = 0;
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

  for(i = 0; i < sects_sz; i++) {
    struct section_list_entry * node;
    printf("sect %s in\n", sects[i]->name);
    node = sects[i]->list;
    while(node) {
      printf("\t%s\n", node->section->name);
      node = node->next;
    }    
  }

  /*
  printf("Loaded labels:\n");
  for(i = 0; i<loaded_files; i++) {
    printf("file %d:\n", i);
    print_labels(label_vecs[i]);
  }
  */

  linker_offset_labels();

/*
  printf("off:\n");
  for(i = 0; i<loaded_files; i++) {
    printf("file %d:\n", i);
    print_labels(label_vecs[i]);
  }
*/
  linker_link();

/*
  f_out = fopen(output_fname, "wb");
  for(i = 0; i<loaded_files; i++) {
    fwrite(images[i], 1, image_sizes[i], f_out);
    printf("%d\t0x%04x\t%s\n", image_sizes[i], image_sizes[i], image_names[i]);
  }
  fclose(f_out);

  for(i = 0; i<loaded_files; i++) {
    free(raw_data[i]);
  }
*/
  return 0;

}