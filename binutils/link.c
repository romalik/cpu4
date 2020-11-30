#include "link.h"
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "labels.h"
#include <stdint.h>
#include <stdlib.h>
#include "robj.h"

#define MAXFILES 10

static uint8_t * images[MAXFILES];
static uint16_t image_sizes[MAXFILES];

static uint8_t * label_vecs[MAXFILES];
static uint16_t label_vec_sizes[MAXFILES];

static uint16_t * label_masks[MAXFILES];
static uint16_t label_mask_sizes[MAXFILES];


uint8_t loaded_files;


void linker_init() {
  memset(images, 0, MAXFILES*sizeof(uint8_t *));
  memset(image_sizes, 0, MAXFILES*sizeof(uint16_t));
  memset(label_vecs, 0, MAXFILES*sizeof(uint8_t *));
  memset(label_vec_sizes, 0, MAXFILES*sizeof(uint16_t));
  memset(label_masks, 0, MAXFILES*sizeof(uint16_t *));
  memset(label_mask_sizes, 0, MAXFILES*sizeof(uint16_t));
  loaded_files = 0;

}


void linker_add_image(uint8_t * image, uint8_t * label_vec, uint8_t * label_mask, uint16_t image_size, uint16_t label_vec_size, uint16_t label_mask_size_bytes) {
  images[loaded_files] = image;
  image_sizes[loaded_files] = image_size;
  label_vecs[loaded_files] = label_vec;
  label_vec_sizes[loaded_files] = label_vec_size;
  label_masks[loaded_files] = (uint16_t *)label_mask;
  label_mask_sizes[loaded_files] = label_mask_size_bytes/2;
  loaded_files++;
}

void linker_offset_labels() {
  uint16_t c_off = 0;
  uint8_t c_image = 1;
  struct label_entry * e;
  while(c_image < loaded_files) {
    c_off += image_sizes[c_image-1];    
    e = (struct label_entry *)label_vecs[c_image];
    while(*(e->name)) {
      if(e->present) e->position += c_off;
      e++;
    }
    c_image++;
  }
}


struct label_entry * find_located_label(uint16_t id, int file) {
  int i;
  char * name;
  struct label_entry * e;
  e = find_label_by_id(id, (struct label_entry *)label_vecs[file]);
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

    e = find_label(name, (struct label_entry *)label_vecs[i]);
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
  int i;
  uint16_t addr;
  uint16_t id;
  for(curr_file = 0; curr_file < loaded_files; curr_file++) {
    for(i = 0; i<label_mask_sizes[curr_file]; i++) {
      addr = label_masks[curr_file][i];
      id = (images[curr_file][addr]) | (images[curr_file][addr+1] << 8);

      e = find_located_label(id, curr_file);
      if(e) {
        images[curr_file][addr] = low(e->position);
        images[curr_file][addr+1] = high(e->position);
      } else {
        exit(1);
      }

    }
  }
}


char * load_file(char * path) {
  FILE *f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *data = malloc(fsize);
  fread(data, 1, fsize, f);
  fclose(f);
  return data;
}


char * output_fname;

char * raw_data[MAXFILES];

int main(int argc, char ** argv) {
  FILE *f_out;
  int i = 0;
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
      raw_data[file_cnt] = load_file(argv[i]);
      file_cnt++;
    }
  }

  linker_init();

  for(i = 0; i<file_cnt; i++) {
    struct robj_header * header = (struct robj_header *)raw_data[i];
    if(memcmp(header->signature,"robj",4)) {
      printf("Bad file %d\n", i);
      exit(1);
    }

    obj_size = ((uint16_t)header->obj_size_h << 8) | ((uint16_t)header->obj_size_l);
    label_vec_size = ((uint16_t)header->label_vec_size_h << 8) | ((uint16_t)header->label_vec_size_l);
    label_mask_size = ((uint16_t)header->label_mask_size_h << 8) | ((uint16_t)header->label_mask_size_l);

    linker_add_image(
      raw_data[i] + sizeof(struct robj_header), 
      raw_data[i] + sizeof(struct robj_header) + obj_size,
      raw_data[i] + sizeof(struct robj_header) + obj_size + label_vec_size,
      obj_size,
      label_vec_size,
      label_mask_size
      );
  }

  linker_offset_labels();

/*
  printf("off:\n");
  for(i = 0; i<loaded_files; i++) {
    printf("file %d:\n", i);
    print_labels(label_vecs[i]);
  }
*/
  linker_link();


  f_out = fopen(output_fname, "wb");
  for(i = 0; i<loaded_files; i++) {
    fwrite(images[i], 1, image_sizes[i], f_out);
  }
  fclose(f_out);

  for(i = 0; i<loaded_files; i++) {
    free(raw_data[i]);
  }

  return 0;

}