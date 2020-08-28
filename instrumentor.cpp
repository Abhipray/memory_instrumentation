// https://blog.kummerlaender.eu/article/notes_on_function_interposition_in_cpp/

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdbool.h>
#include <stdlib.h>

extern "C" {
void *malloc(size_t size);
int rand(void);
}

#include <cstring>
#include <iostream>

// namespace {
// orig_malloc_f_type orig_malloc;
// orig_free_f_type orig_free;
// orig_calloc_f_type orig_calloc;
// orig_realloc_f_type orig_realloc;
// }  // namespace

static bool is_initialized_ = false;

int rand(void) {
  typedef int (*orig_rand_f_type)();
  orig_rand_f_type orig_rand;
  orig_rand = (orig_rand_f_type)dlsym(RTLD_NEXT, "rand");
  // std::cout << ("WLAA");
  // return 43;
  return orig_rand();  // the most random number in the universe
}

// static void initialize(void) {
//   // TODO: Convert to define
//   if (!is_initialized_) {
//     // const void* symbol_address{dlsym(RTLD_NEXT, "malloc")};

//     // std::memcpy(&actual_function, &symbol_address,
//     sizeof(symbol_address)); orig_malloc =
//     (orig_malloc_f_type)dlsym(RTLD_NEXT, "malloc");
//     // orig_free = (orig_free_f_type)dlsym(RTLD_NEXT, "free");
//     // orig_calloc = (orig_calloc_f_type)dlsym(RTLD_NEXT, "calloc");
//     // orig_realloc = (orig_realloc_f_type)dlsym(RTLD_NEXT, "realloc");
//     is_initialized_ = true;
//   }
// }

// void* malloc(size_t size) {
//   // printf("malloc intercepted");
//   // initialize();
//   typedef void* (*orig_malloc_f_type)(size_t size);
//   // (void*)(orig_malloc*)(size_t){};
//   orig_malloc_f_type orig_malloc;
//   orig_malloc = (orig_malloc_f_type)dlsym(RTLD_NEXT, "malloc");

//   // const void* symbol_address = dlsym(RTLD_NEXT, "malloc");
//   // std::memcpy(&orig_malloc, &symbol_address, sizeof(symbol_address));
//   return orig_malloc(size);
// }

void *malloc(size_t size) {
  static void *(*mallocp)(size_t size);
  char *error;
  void *ptr;
  typedef void *(*orig_malloc_f_type)(size_t size);

  /* get address of libc malloc */
  if (!mallocp) {
    mallocp = (orig_malloc_f_type)dlsym(RTLD_NEXT, "malloc");
    if ((error = dlerror()) != NULL) {
      fputs(error, stderr);
      exit(1);
    }
  }
  ptr = mallocp(size);
  printf("malloc(%d) = %p\n", size, ptr);
  return ptr;
}