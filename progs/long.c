#include "stdlib.h"

int main() {
/*
   long a = 4096;
   long b = 256;
   long c = 1024;
   long d = 512;
*/

   long a = 60;
   long b = 40;
   long c = 10;
   long d = 120;

   long r1;
   long r2;

   int ai = 60;
   int bi = 40;
   int ci = 10;
   int di = 120;

   int ri1;
   int ri2;

   if(a*b == c*d*2) {
      printf("long Equal\n");
   } else {
      printf("long Not equal\n");
   }

   r1 = a*b;
   r2 = c*d*2;

   if(r1 == r2) {
      printf("long asgn Equal\n");
   } else {
      printf("long asgn Not equal\n");
   }

   if(ai*bi == ci*di*2) {
      printf("int Equal\n");
   } else {
      printf("int Not equal\n");
   }

   if(ri1 == ri2) {
      printf("int asgn Equal\n");
   } else {
      printf("int asgn Not equal\n");
   }

   return 0;
}
