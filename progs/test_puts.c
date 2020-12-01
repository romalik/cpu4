void putc(char c) {
  *(char *)(0x4000) = c;
}

void puts(char *s) {
  while(*s) {
    putc(*s);
    s++;
  }
}


int main() {
  char * str = "test string\nfck yeah\n";
  puts(str);

  return 0;
}
