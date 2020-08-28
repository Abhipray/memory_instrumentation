// #define _GNU_SOURCE
// #include <dlfcn.h>
// #include <stdio.h>
// #include <stdlib.h>

// int rand() {
//   typedef int (*orig_rand_f_type)();
//   orig_rand_f_type orig_rand;
//   orig_rand = (orig_rand_f_type)dlsym(RTLD_NEXT, "rand");
//   printf("sssss");
//   return orig_rand();  // the most random number in the universe
// }

// void *malloc(size_t size) {
//   static void *(*mallocp)(size_t size) = NULL;
//   char *error;
//   void *ptr;
//   typedef void *(*orig_malloc_f_type)(size_t size);

//   /* get address of libc malloc */
//   if (!mallocp) {
//     mallocp = (orig_malloc_f_type)dlsym(RTLD_NEXT, "malloc");
//     if ((error = dlerror()) != NULL) {
//       fputs(error, stderr);
//       exit(1);
//     }
//   }
//   ptr = mallocp(size);
//   printf("malloc(%lu) = %p\n", size, ptr);
//   return ptr;
// }

#define _GNU_SOURCE

#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

static void* (*real_malloc)(size_t) = NULL;

static void mtrace_init(void) {
  real_malloc = dlsym(RTLD_NEXT, "malloc");
  if (NULL == real_malloc) {
    fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
  }
}

// void *malloc(size_t size) {
//   if (real_malloc == NULL) {
//     mtrace_init();
//   }

//   void *p = NULL;
//   fprintf(stderr, "malloc(%d) = ", size);
//   p = real_malloc(size);
//   fprintf(stderr, "%p\n", p);
//   return p;
// }

void* malloc(size_t size) {
  write(STDOUT_FILENO, "malloc\n", 7);
  // printf("malloc\n"); leads to Segmentation Fault on MacOSX
  return ((void* (*)(size_t))dlsym(RTLD_NEXT, "malloc"))(size);
}

void free(void* ptr) {
  write(STDOUT_FILENO, "free\n", 5);
  ((void (*)(void*))dlsym(RTLD_NEXT, "free"))(ptr);
}