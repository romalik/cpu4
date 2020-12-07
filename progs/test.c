#include "stdlib.h"

void __puts(char *s) {
  while(*s) {
    putc(*s);
    s++;
  }
}


int main() {
   __puts("test\n");   
   return 0;
}