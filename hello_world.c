// https://rafalcieslak.wordpress.com/2013/04/02/dynamic-linker-tricks-using-ld_preload-to-cheat-inject-features-and-investigate-programs/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

int main() {
  // setlocale(LC_ALL, "en_US.UTF-8");
  fprintf(stderr, "Starting app\n");
  void* ptr = malloc(11);
  malloc(12);
  malloc(13);
  // srand(time(NULL));
  // printf("Helloooooo %d!\n", rand());
  // fprintf(stderr, "app exit\n");
  // free(ptr);
}