#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include "labels.h"
#include "util.h"
#include "token.h"
#include "keywords.h"
#include "robj.h"

#define panic(msg) panic_full((msg), __FILE__, __LINE__, current_line, token)

uint16_t current_pos = 0;

char image[1024];
char label_vec[1024];
char label_usage_list[1024];

void emit(uint8_t opcode) {
  printf("put 0x%02x at 0x%04x\n", opcode, current_pos);
  image[current_pos] = opcode;
  current_pos++;
}

char expect_args = 0;

void gen_instruction() {
  char keyword_id;
  char arg_id;
  uint8_t opcode = 0;
  keyword_id = find_keyword(opcodes_0, token);
  if(keyword_id > 15) {
    panic("Bad keyword");
  }
  opcode = keyword_id << 4;
  printf("kword id %d\n", keyword_id);
  switch(keyword_id) {
    case 0:
    case 13:
      emit(opcode);
      break;
    case 4:
      //expect 2 bytes args
      expect_args++;
    case 3:
      //expect 1 byte arg
      expect_args++;
    case 1:
    case 2:
    case 5:
    case 6:
    case 7:
    case 8:
      //no args expected
      get_next_token();
      if(eof_hit) {
        panic("Unexpected EOF");
      }
      arg_id = find_keyword(args, token);
      if(arg_id > 15) {
        panic("Bad argument");
      }
      opcode |= arg_id;
      emit(opcode);
      return;
    case 14:
      //call
      emit(opcode);
      expect_args = 2;
      return;
    case 11:
      //jmp, expect adr
      expect_args = 2;
    case 12:
      //jmpX
      get_next_token();
      //COND_MASK (((!CF) << 3) | (CF << 2) | ((!ZF) << 1) | (ZF))
      if(!strcmp(token,"z")) {
        arg_id = 1;
      } else if(!strcmp((token), "nz")) {
        arg_id = 2;
      } else if(!strcmp((token), "c")) {
        arg_id = 4;
      } else if(!strcmp((token), "nc")) {
        arg_id = 8;
      } else {
        arg_id = 15;
        unget_token();
      }
      opcode |= arg_id;
      emit(opcode);
      return;

      break;
    case 9:
    case 10:
      //alu and cmp
      get_next_token();
      if(eof_hit) {
        panic("Unexpected EOF");
      }
      arg_id = find_keyword(alu_args, token);
      if(arg_id > 15) {
        panic("Bad argument");
      }
      opcode |= arg_id;
      emit(opcode);
      return;
      break;
    case 15:      
      //ext
      get_next_token();
      keyword_id = find_keyword(opcodes_1, token);
      if(keyword_id < 16) {
        arg_id = 1;
        opcode |= arg_id;
        emit(opcode);

        switch(keyword_id) {
          case 14:
          //info
            emit(keyword_id << 4);
            expect_args = 1;
            break;
          case 15:
          //halt
            emit(keyword_id << 4);
            break;
          case 2:
          default:
          break;
        }

      } else {
        panic("Bad ext instruction");
      }
      break;
    default:
      break;
  }

}

void assemble() {
  char token_length = 0;
  uint8_t byte_literal;
  uint16_t word_literal;

  while(1) {
    token_length = get_next_token();
    printf("token '%s'\n", token);
    if(token[0] == ';') {
      skip_comment();
      continue;
    }

    if(eof_hit) break;

    if(token[token_length - 1] == ':') {
      printf("Got label %s\n", token);
      token[token_length - 1] = 0;
      mark_label_position(token, current_pos);
    } else if(token[0] == '.') {
      //directive

      if(expect_args) panic("Arguments expected");

      if(!strcmp(&token[1], "org")) {
        get_next_token();
        current_pos = strtol(token,0,0);
      } else if(!strcmp(&token[1], "byte")) {
        expect_args = 1;
      } else if(!strcmp(&token[1], "word")) {
        expect_args = 2;
      } else if(!strcmp(&token[1], "skip")) {
        get_next_token();
        current_pos += strtol(token,0,0);
      }

    } else if(token[0] == '$') {
      //label
      uint16_t label_id;
      label_id = mark_label_use(&token[1], current_pos);
      emit(high(label_id));
      emit(low(label_id));
      expect_args = 0;

    } else if(token[0] == '\'') {
      //literal char
      if(expect_args > 1) {
        emit(0);
      }
      if(expect_args) {
        emit(token[1]);
      }
      if(!expect_args) {
        panic("Unexpected literal char!");
      }
      expect_args = 0;
    } else if(token[0] >= '0' && token[0] <= '9') {
      //literal
      word_literal = strtol(token,0,0);
      if(expect_args > 1) {
        emit(high(word_literal));
      }
      if(expect_args) {
        emit(low(word_literal));
      }
      if(!expect_args) {
        panic("Unexpected literal!");
      }
      expect_args = 0;
    
    } else  {
      if(expect_args) {
        panic("Arguments expected");
      }
      gen_instruction();
    }

  }
  if(expect_args) {
    panic("Hit eof expecting args");
  }

}

void print_label_usage(uint16_t size) {
  int i;
  for(i = 0; i<size/2; i++) {
    printf("Label used on 0x%04x\n", ((uint16_t *)label_usage_list)[i]);
  }

}

char * infile_name;
char * outfile_name;

int main(int argc, char ** argv) {
  FILE * infile;
  FILE * outfile;
  struct robj_header header;
  if(argc < 3) {
    printf("Usage: %s infile outfile\n", argv[0]);
    exit(1);
  }

  infile_name = argv[1];
  outfile_name = argv[2];

  infile = fopen(infile_name, "r");

  parser_set_file(infile);

  memset(label_vec, 0, 1024);
  set_label_vec(label_vec);
  set_label_usage_list(label_usage_list);

  assemble();
  print_labels(label_vec);
  print_label_usage(get_label_usage_list_size());
  hexdump(image, current_pos);

/*
  linker_init();

  linker_add_image(image, label_vec, label_usage_list, current_pos, get_label_vec_size(), get_label_usage_list_size());

  linker_offset_labels();
  linker_link();
*/
  hexdump(image, current_pos);

  fclose(infile);

  outfile = fopen(outfile_name, "wb");

  uint16_t label_vec_size = get_label_vec_size();
  uint16_t label_mask_size = get_label_usage_list_size();

  memcpy(header.signature, "robj", 4);
  header.obj_size_h = high(current_pos);
  header.obj_size_l = low(current_pos);
  header.label_vec_size_h = high(label_vec_size);
  header.label_vec_size_l = low(label_vec_size);
  header.label_mask_size_h = high(label_mask_size);
  header.label_mask_size_l = low(label_mask_size);

  fwrite(&header, 1, sizeof(struct robj_header), outfile);
  fwrite(image, 1, current_pos, outfile);
  fwrite(label_vec, 1, label_vec_size, outfile);
  fwrite(label_usage_list, 1, label_mask_size, outfile);

  fclose(outfile);
  
}