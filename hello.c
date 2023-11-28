int main() {
  int a = 2;
  if (a) {
    a = a + 1;
  } else a = 0;  // 在实际写 C/C++ 程序的时候别这样, 建议 if 的分支全部带大括号
  return a;
}

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