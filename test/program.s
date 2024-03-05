test1:
test2:
  li  x1, 3
  add x1, x2, x3
  j   20
  j   test1
  j   test2

test3:
  bge x3, x31, test1
