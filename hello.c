// int half(int x, int y) {
  
//   return x+y;
// }

// void f() {}

// int main() {
//   int x = 10;
//   int y =half(1, x);
//   int z = x + y;
//   return z;
// }

int x;
const int y = 10;
const int z = y + 1;
int init = 1;

int main() {
  putint(x);
  putch(32);
  putint(y);
  putch(32);
  putint(z);
  putch(32);
  putint(init);
  putch(10);
  return 0;
}


// sp+48

// sp

/**
fun @main(): i32 {
  or
@1 - 1
%entry:
  
  or:
  ---
  %a = alloc i32
  store 1, %a
  %0 = gt 2, 1
  br %0, %branch1, %branch0
%branch0:
  %1 = gt 2, 0
  br %1, %branch1, %branch2
%branch2:
  stroe 0, %a
  jumo brach2;
%bracnh1:  
  %2 = load %a
  
  and:
  ----
  %a = alloc i32
  store 0 %a
  %0 = gt 2, 1
  br %0, %branch1, %branch0
%branch1:
  %1 = gt 2, 0
  br %1, %branch2, %branch0
%branch2:
  store 1, %a
  jump %branch0
%branch0:
  %2 = load %a

  




fun @main(): i32 {
%entry:
  %0 = gt 2, 1
  %1 = gt 2, 0
  %3 = or %0, %1
  %2 = ne 0, %3
  br %2, %branch0, %branch1
%branch0:
  ret 1
%branch1:
  ret 0
}

*/