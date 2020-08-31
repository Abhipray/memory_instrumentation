/**
 *  This app mallocs memory in two separate threads. Used for testing LD_PRELOAD
 * instrumentation.
 *
 */
#include <locale.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void* test_thread(void* vargp) {
  for (int i = 1; i < 100; i++) {
    for (int j = 0; j < 10; j++) {
      size_t bytes = rand() % 4096;
      void* ptr = malloc(i);
    }
    sleep(5);
  }
  return NULL;
}

int main() {
  srand(time(NULL));
  pthread_t thread1, thread2;
  pthread_create(&thread1, NULL, test_thread, NULL);
  pthread_create(&thread2, NULL, test_thread, NULL);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
}