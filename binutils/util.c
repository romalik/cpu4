#include "util.h"
#include "labels.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void panic_full(char * msg, char * c_file, int c_line, int current_line, char * token) {
  printf("Error (%s:%d): \"%s\" at line %d near \"%s\"\n", c_file, c_line, msg, current_line, token);
  exit(1);
}

#define panic(msg) panic_full((msg), __FILE__, __LINE__)


char find_keyword(char * kw[], char * str) {
  char i;
  for(i = 0; i<16; i++) {
    if(!strcmp(kw[i], str)) {
      return i;
    }
  }
  return 16;
}


void hexdump(uint8_t * data, uint16_t length) {
  uint16_t i = 0;
  uint8_t j = 0;
  printf("Image:\n");
  for(i = 0; i<length; i+=16) {
    printf("0x%04x\t", i);
    for(j = 0; j<16; j++) {
      printf("%02x ", (data[i+j] & 0xff));
      if(j == 7) printf(" ");
    }
    printf("\n");
  }
}



