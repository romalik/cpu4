void putc(char c) {
  *(char*)0x4000 = c;
}

int main() {
  putc('H');
  putc('i');
  putc('!');
  putc('\n');

}
