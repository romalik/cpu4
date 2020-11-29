int a(int arg) {
  return arg + 1;
}

int b(int arg) {
  return arg + 2;
}

int c(int arg1, int arg2) {
  return arg1+arg2;
}


int a_100(int);
int b_200(int);
int c_101_102(int,int);
int a_b_c(int);

int main() {
  int v1 = 1;
  int v2 = 2;
  int v3;

  char v4;
 
  v3 = v2 = v1 + 10;


  v4 = v3 + 123;

  v3 = c(a_100(100),a(b_200(200) + c_101_102(101,102)));
}

