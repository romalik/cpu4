void putc(char c) {
  *(char*)0x4000 = c;
}



void less() {
    putc('<');
    putc('\n');
}
void lesseq() {
    putc('<');
    putc('=');
    putc('\n');
}
void more() {
    putc('>');
    putc('\n');
}
void moreeq() {
    putc('>');
    putc('=');
    putc('\n');
}
void eq() {
    putc('=');
    putc('\n');
}
void ne() {
    putc('!');
    putc('\n');
}
void line() {
    putc('-');
    putc('-');
    putc('-');
    putc('\n');

}

int main() {
  //int a = 3;
  int a = 7;
  int b = 0;
//  for(b = 0; b<3; b++) {
  for(b = -4; b<-1; b++) {
    line();

    if(a + b > 4) {
      more();
    }
    if(a + b >= 4) {
      moreeq();
    }

    if(a + b < 4) {
      less();
    }
    if(a + b <= 4) {
      lesseq();
    }

    if(a + b == 4) {
      eq();
    }

    if(a + b != 4) {
      ne();
    }


    line();
  }


}
