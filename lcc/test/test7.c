int func_int() {
  return 0;
}


char func_char() {
  return 0;
}


long func_long() {
  return 0;
}



int extern_func_int();
char extern_func_char();
long extern_func_long();

int main() {
  int a;
  char b;

  a = extern_func_int();
  a = extern_func_long();
  b = extern_func_char();

 
}
