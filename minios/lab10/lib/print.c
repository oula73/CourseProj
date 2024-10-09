#include "print.h"
#include "sbi.h"

void puts(char *s) {
  //#error "Still have unfilled code!"
  // unimplemented
  while (*s != 0)
  {
    sbi_ecall(0x1, 0x0, *s++, 0, 0, 0, 0, 0);
  }
}

void puti(int x) {
  //#error "Still have unfilled code!"
  // unimplemented
  char s[10];
  int len = -1;
  while (x)
  {
    s[++len] = x % 10 + 0x30;
    x /= 10;
  }
  while (len >= 0)
  {
    sbi_ecall(0x1, 0x0, s[len--], 0, 0, 0, 0, 0);
  }
}
