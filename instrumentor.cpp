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
    overall_allocations += 1;
    time_t t = time(nullptr);
    allocs.insert(pair<void*, pair<size_t, time_t>>(ptr, {size, t}));
  }

private:
  map<void*, pair<size_t, time_t>> allocs;
  uint32_t overall_allocations;

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
  fprintf(stderr, "Malloc %zu\n", size);
  void* ptr = orig_malloc_(size);
  
  // Flag to track invocation by instrumentor
  static bool app_invocation = true;

  if (app_invocation && ptr != nullptr) {
    app_invocation = false;
    // Any subsequent malloc calls will be made by the instrumentor
    instrumentor.malloc(ptr, size);
    app_invocation = true;
  }
  

  return ptr;
}