#ifndef UTIL_H__
#define UTIL_H__
#include <stdint.h>


void panic_full(char * msg, char * c_file, int c_line, int current_line, char * token);


char find_keyword(char * kw[], char * str);

#define low(x) (((uint8_t *)&x)[0])
#define high(x) (((uint8_t *)&x)[1])

void print_labels(uint8_t * label_vec);
void hexdump(uint8_t * data, uint16_t length);


#endif