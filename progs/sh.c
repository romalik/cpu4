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
   int a = 3;
   int b = 2;
   int d = 0;
   char c1 = 5;
   char c2 = 10;

   printhex(0x1234);
   a = 3;
   b = 2;
   puts("\n0x3*0x2 = 0x6:\n");
   printhex(a * b);
   puts("\n");
   a = 0x133;
   b = 0x24;
   puts("\n0x133*0x24 = 0x2b2c:\n");
   printhex(a * b);
   puts("\n");

   a = 0x112;
   b = 0x31;
   puts("\n0x112*0x31 = 0x3472:\n");

   __crt_MULI2(a,b);

   printhex(d);
   puts("\n");

   a = 0x6;
   b = 0x2;
   puts("\n0x6/0x2 = 0x3:\n");

   printhex(a/b);
   puts("\n");

   a = 0x3A43;
   b = 0x34D;
   puts("\n0x3a43/0x34d = 0x11:\n");

   printhex(a/b);
   puts("\n");

   puts("\n0x3a43%0x34d = 0x226:\n");

   printhex(a%b);
   puts("\n");

   a = 1234;
   b = 6420;
   puts("1234+6420 = 7654\n");
   printnum(a+b);
   puts("\n");

   puts("Commands:\n");
   for(a = 0; a < N_CMDS; a++) {
      printhex(a); puts(": "); puts(cmds[a]); puts("\n");
   }

   c = 'a';
   a = 10;
   b = 0x34;


   printf("this is a printf test (a 10 0x34 blah) %c %d 0x%02x %s\n", c, a, b, tstr);

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

