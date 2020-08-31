#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
  fprintf(stderr, "Starting app\n");
  srand(time(NULL));
  for (int i = 0; i < 100; i++) {
    size_t bytes = rand() % 8192;
    void* ptr = malloc(bytes);
    sleep(5);
  }
}