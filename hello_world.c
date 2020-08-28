// https://rafalcieslak.wordpress.com/2013/04/02/dynamic-linker-tricks-using-ld_preload-to-cheat-inject-features-and-investigate-programs/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  printf("Ola! \n");
  srand(time(NULL));
  printf("Helloooooo %d!\n", rand());
}