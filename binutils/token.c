#include <stdint.h>
#include <stdio.h>
#include "util.h"
#include "token.h"
#include <ctype.h>
#include <string.h>

#define panic(msg) panic_full((msg), __FILE__, __LINE__, current_line, token)

int current_line = 0;
char eof_hit = 0;

char token[64];

static FILE * infile;

static uint8_t fl_unget = 0;


static int b_getc_last = 0;
int b_getc(FILE * fp) {
  b_getc_last = getc(fp);
  return b_getc_last;
}

void parser_set_file(FILE * f) {
  infile = f;
}

void unget_token() {
  fl_unget = 1;
}

int get_quoted_text() {
  int c;
  static int inside_quoted = 0;
  while(1) {
    if((c = b_getc(infile)) == EOF) {
      panic("quote expected, eof hit\n");
    }
    if(c == '"') {
      if(!inside_quoted) {
        inside_quoted = 1;
        continue;
      } else {
        inside_quoted = 0;
        return -1;
      }
    }

    if(inside_quoted) {
      if(c == '\\') {
        if((c = b_getc(infile)) == EOF) {
          panic("escape expected, eof hit\n");
        }
        switch(c) {
          case 'n': return '\n';
          case 't': return '\t';
          case 'a': return '\a';
          case 'b': return '\b';
          case 'r': return '\r';
          case '0': return 0;
          default: return c;
        }
      } else {
        return c;
      }
    }
  }
}

char get_next_token() {
  int c;
  char len;
  if(fl_unget) {
    fl_unget = 0;
    return strlen(token);
  }
  len = 0;
  while(1) {

    if((c = b_getc(infile)) == EOF) {
      token[len] = 0;
      if(len == 0)
        eof_hit = 1;
        //printf("eof\n");
      return len;
    }


    if(isspace(c) || c == '\n') {
      if(c == '\n') current_line++;
      if(len == 0) {
        continue;
      } else {
        token[len] = 0;
        return len;
      }
    } else {
      token[len] = (char)c;
      len++;
    }
  }
}

void skip_comment() {
  int c;
  while(1) {
    if(b_getc_last == '\n') {
      //one word comment
      return;
    }
    if((c = b_getc(infile)) == EOF) {
      eof_hit = 1;
      return;
    }
    if(c == '\n') {
      current_line++;
      return;
    }
  }
}
