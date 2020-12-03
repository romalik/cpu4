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
      if(!strcmp(cmds[i], s)) {
         return i;
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

char buffer[30];

int main() {
   char c;
   int cmd_idx;
   int a;
   int b = 5;

   //printnum(a*b);

   buffer[b] = '0';

   for(b = 0; b<10; b++) {
      //a = 9-b;
      //buffer[b] = '0';//+b;
   }
   buffer[10] = 0;
   puts(buffer);

   for(a = 15; a > 0; a--) {
      for(b = 0; b<a; b++) {
         putc('*');
      }
      putc('\n');
   }

   while(1) {
      puts("> ");
      getline();
      puts("\ncmd: ");
      puts(in_str);
      puts("\n");

      if(!strcmp(in_str, "test")) {
         puts(">> Test successful\n");
         funcs[1]();
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

