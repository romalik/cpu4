char buffer[100];


int fn(int a,int b) {
 return a;
}

int main() {
  char c;
  int cmd_idx;
  int a;
  int b = 5;
  buffer[10-b] = '0'+b+buffer[b] + fn(buffer[b+1], b-1);
//  buffer[b] = '0';
}
