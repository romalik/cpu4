#ifndef TOKEN_H__
#define TOKEN_H__

extern int current_line;
extern char eof_hit;

extern char token[];

void parser_set_file(FILE * f);

char get_next_token();
void skip_comment();
void unget_token();
int get_quoted_text();

#endif
