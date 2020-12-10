#include "stdlib.h"

int main() {
   long long_var = -1234567890;
   long long_var2 = 10;
   long long_var3;
   int a = -12345;
   int a2 = 10;
   int a3;
   printf("1234567890: %l\n", long_var);
   printf("1234567890/-10: %l\n", long_var/(-10));
   long_var3 = long_var / long_var2;
   printf("1234567890/-10: %l\n", long_var3);

   printf("12345: %d\n", a);

   printf("12345/-10: %d\n", a/(-10));
   printf("a2: %d\n", a2);
   a3 = a / a2;
   printf("12345/-10: %d\n", a3);


   return 0;
}
