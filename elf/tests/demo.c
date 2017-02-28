int a;
extern int b;
int do_something(int v) {
  b = a;
  a = v;
  return b;
}
