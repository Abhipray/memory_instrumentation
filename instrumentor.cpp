// https://blog.kummerlaender.eu/article/notes_on_function_interposition_in_cpp/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <stdbool.h>
#include <stdlib.h>
#include <vector>
#include <time.h>

extern "C" {
#include "instrumentor.h"
}

#include <cstring>
#include <iostream>
#include <map>

using namespace std;

class Instrumentor {
  public:
  
  Instrumentor(){
    
  }

  void malloc(void* ptr, size_t size){
    if (app_invocation){
      app_invocation = false;
      overall_allocations += 1;
      time_t t = time(nullptr);
      allocs.insert(pair<void*, pair<size_t, time_t>>(ptr, {size, t}));
      app_invocation = true;
    } else {
      fprintf(stderr, "Ignoring malloc instrumentation");
    }
  }

  void free(void *ptr) {
    allocs.erase(ptr);
  }

  void realloc(void* old_ptr, void * new_ptr, size_t new_size){
    if (app_invocation){
      app_invocation = false;
      allocs.erase(old_ptr);
      time_t t = time(nullptr);
      allocs.insert(pair<void*, pair<size_t, time_t>>(new_ptr, {new_size, t}));
      app_invocation = true;
    } else {
      fprintf(stderr, "Ignoring malloc instrumentation");
    }
  }

private:
  map<void*, pair<size_t, time_t>> allocs;
  uint32_t overall_allocations;
  // Flag to track calls by instrumentor
  bool app_invocation = true;

};

namespace {
orig_malloc_f_type orig_malloc_;
orig_free_f_type orig_free_;
orig_calloc_f_type orig_calloc_;
orig_realloc_f_type orig_realloc_;
Instrumentor instrumentor;
}  // namespace


static bool is_initialized_ = false;


static void initialize(void) {
  // TODO: Convert to define
  if (!is_initialized_) {
    orig_malloc_ =
    (orig_malloc_f_type)dlsym(RTLD_NEXT, "malloc");
    orig_free_ = (orig_free_f_type)dlsym(RTLD_NEXT, "free");
    orig_calloc_ = (orig_calloc_f_type)dlsym(RTLD_NEXT, "calloc");
    orig_realloc_ = (orig_realloc_f_type)dlsym(RTLD_NEXT, "realloc");
    instrumentor = Instrumentor();
    is_initialized_ = true;
  }
}


void *malloc(size_t size) {
  initialize();

  void* ptr = orig_malloc_(size);
  
  fprintf(stderr, "Malloc %zu %zu\n", size, (size_t)ptr);

  if (ptr != nullptr) {
    instrumentor.malloc(ptr, size);
  }
  

  return ptr;
}

void free(void* ptr){
  initialize();
  fprintf(stderr, "Free %zu\n", (size_t)ptr);
  orig_free_(ptr);
  instrumentor.free(ptr);
}

void* realloc(void* ptr, size_t size){
  initialize();
  fprintf(stderr, "Realloc %zu %zu\n", (size_t)ptr, size);
  void * new_ptr = orig_realloc_(ptr, size);
  instrumentor.realloc(ptr, new_ptr, size);
}

void* calloc(size_t num, size_t size) {
  initialize();
  void* ptr = orig_calloc_(num, size);
  fprintf(stderr, "calloc %zu\n", (size_t)ptr);
  instrumentor.malloc(ptr, size);
}