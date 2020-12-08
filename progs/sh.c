#include "stdlib.h"

#define MAX_LENGTH 127

void test_func() {
   puts("this is a test func\n");   
}

void test_func_2() {
   puts("another test func\n");   
}

#define N_CMDS 2

void (*funcs[])() = {test_func, test_func_2};
char * cmds[] = {"test1", "test2"};


int get_cmd_idx(char * s) {
   int i = 0;
   for(i = 0; i<N_CMDS; i++) {
      puts("comparing ");
      puts(cmds[i]);
      puts("\n");
      if(!strcmp(cmds[i], s)) {
         puts("Success at "); printhex(i); puts("\n");
         return i;
      } else {
         puts("Fail at "); printhex(i); puts("\n");
      }
   }
   return -1;
}

char in_str[MAX_LENGTH];
void getline() {
   char c;
   int count = 0;
   char * s = in_str;
   while(1) {
      c = getc();
      if(!c) continue;
      if(c == '\n') {
         *s = 0;
         return;
      } else {
         if(count < MAX_LENGTH - 1) {
            putc(c);
            *s = c;
            s++;
            count++;
         }
      }
   }

}

char * aaa = "blah blah\n";

char buffer[30];

extern int __crt_MULI2(int,int);

int main() {
   char c;
   char * tstr = "blah";
   int cmd_idx;
   long long_var = 1234567;
   int a = 3;
   int b = 2;
   int d = 0;
   char c1 = 5;
   char c2 = 10;

   printf("print test (12 23 34 'a' 'b' 'c' 0x3f 0x5c 'blah')%d %d %d %c %c %c 0x%02x 0x%02x '%s'\n", 12,23,34,'a','b','c',0x3f,0x5c,tstr);


   a = 1231;
   b = 38;

   printf("1231*18 = %d (=22158)\n", a*b);

   a = 45;
   b = 30;

   printf("45*30 = %d (=1350)\n", a*b);

//   printf("long: %l\n", long_var);

   while(1) {
      puts("> ");
      getline();
      puts("\ncmd: ");
      puts(in_str);
      puts("\n");

      if(!strcmp(in_str, "test")) {
         puts(">> Test successful\n");
      }

      cmd_idx = get_cmd_idx(in_str);

      if(cmd_idx < 0) {
         puts("unknown command\n");
      } else {
         funcs[cmd_idx]();
      }
   }
   return 0;
}

