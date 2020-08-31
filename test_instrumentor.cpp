#include <cassert>
#include <iostream>
#include <map>

#include "instrumentor.h"

using namespace std;

// https://stackoverflow.com/questions/2193544/how-to-print-additional-information-when-assert-fails
#define ASSERT(left, operator, right)                                                              \
  {                                                                                                \
    if (!((left) operator(right))) {                                                               \
      cerr                                                                                         \
          << "ASSERT FAILED: " << #left                                                            \
          << #                                                                                     \
             operator<< #right << " @ " << __FILE__ << " (" << __LINE__ << ")"                     \
                                                                           "."                     \
                                                                           " " << #left << "=" <<( \
                                                                               left)               \
          << "; " << #right << "=" << (right) << std::endl;                                        \
    }                                                                                              \
  }

void verify_sizes_and_timing(size_t expected_size, size_t min_size,
                             size_t max_size) {
  freeze_instrumentor();

  map<void*, pair<size_t, time_t>> allocs;
  get_alloc_map(&allocs);

  ASSERT(allocs.size(), ==, expected_size);

  // Verify sizes and timing
  time_t prev_time = 0;
  int i = 1;

  for (auto const& x : allocs) {
    pair<size_t, time_t> size_and_time = x.second;
    size_t size = size_and_time.first;
    time_t alloc_time = size_and_time.second;

    ASSERT(alloc_time, >, prev_time);
    ASSERT(size, >=, min_size);
    ASSERT(size, <=, max_size);
    i++;
  }
  unfreeze_instrumentor();
}

int main() {
  /* Test 1: Call malloc 10 times and check allocs */
  for (int i = 1; i <= 10; i++) {
    malloc(i);
  }

  verify_sizes_and_timing(10, 1, 10);
  fprintf(stderr, "Test 1 (malloc): Pass\n");

  /* Test 2: Call free on all malloced pointers */
  freeze_instrumentor();
  map<void*, pair<size_t, time_t>> allocs;
  get_alloc_map(&allocs);

  for (auto const& x : allocs) {
    pair<size_t, time_t> size_and_time = x.second;
    size_t size = size_and_time.first;
    time_t alloc_time = size_and_time.second;
    free(x.first);
  }
  get_alloc_map(&allocs);
  ASSERT(allocs.size(), ==, 0);

  fprintf(stderr, "Test 2 (free): Pass\n");
  unfreeze_instrumentor();

  /** Test 3: Call calloc 10 times and check allocs */
  for (int i = 1; i <= 20; i++) {
    calloc(1, i);
  }
  verify_sizes_and_timing(20, 1, 20);
  fprintf(stderr, "Test 3 (calloc): Pass\n");

  /** Test 4: Call realloc on first element and check allocs */
  freeze_instrumentor();
  get_alloc_map(&allocs);
  void* ptr = allocs.begin()->first;
  size_t orig_size = allocs.begin()->second.first;

  unfreeze_instrumentor();

  void* new_ptr = realloc(ptr, 100);

  freeze_instrumentor();
  get_alloc_map(&allocs);
  // Ensure that there is a new element in the map with 100 bytes and no element
  // with the original number of bytes
  bool success = false;
  for (auto const& x : allocs) {
    pair<size_t, time_t> size_and_time = x.second;
    size_t size = size_and_time.first;
    if (size == 100 && x.first == new_ptr) {
      success = true;
    }
    if (size == orig_size) {
      success = false;
      break;
    }
  }
  ASSERT(success, ==, true);

  fprintf(stderr, "Test 4 (realloc): Pass\n");
  unfreeze_instrumentor();
}