#include <gb/gb.h>
#include <stdio.h>

int main(void) {
  printf("Hello World!");

  while (1) {
    wait_vbl_done();
  }
}
