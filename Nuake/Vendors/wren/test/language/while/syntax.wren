// Single-expression body.
var c = 0
while (c < 3) System.print(c = c + 1)
// expect: 1
// expect: 2
// expect: 3

// Block body.
var a = 0
while (a < 3) {
  System.print(a)
  a = a + 1
}
// expect: 0
// expect: 1
// expect: 2
