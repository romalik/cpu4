#ifndef LINK_H__
#define LINK_H__

#include <stdint.h>

void linker_init();
void linker_add_image(uint8_t * image, uint8_t * label_vec, uint8_t * label_mask, uint16_t image_size, uint16_t label_vec_size, uint16_t label_mask_size);
void linker_offset_labels();
void linker_link();

#endif