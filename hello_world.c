// https://rafalcieslak.wordpress.com/2013/04/02/dynamic-linker-tricks-using-ld_preload-to-cheat-inject-features-and-investigate-programs/

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
  // setlocale(LC_ALL, "en_US.UTF-8");
  fprintf(stderr, "Starting app\n");
  void* ptr = malloc(1024);
  malloc(15);
  sleep(5);
  malloc(13);
  // srand(time(NULL));
  // printf("Helloooooo %d!\n", rand());
  // fprintf(stderr, "app exit\n");
  // free(ptr);
}