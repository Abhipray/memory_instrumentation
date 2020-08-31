// References:
// https://blog.kummerlaender.eu/article/notes_on_function_interposition_in_cpp/
// https://rafalcieslak.wordpress.com/2013/04/02/dynamic-linker-tricks-using-ld_preload-to-cheat-inject-features-and-investigate-programs/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
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
  Instrumentor() { last_print_time = time(nullptr); }

  void malloc(void* ptr, size_t size) {
    if (app_invocation) {
      app_invocation = false;
      overall_allocations += 1;
      time_t t = time(nullptr);
      allocs.insert(pair<void*, pair<size_t, time_t>>(ptr, {size, t}));
      app_invocation = true;
    }
    print_stats();
  }

  void free(void* ptr) {
    allocs.erase(ptr);
    print_stats();
  }

  void realloc(void* old_ptr, void* new_ptr, size_t new_size) {
    if (app_invocation) {
      app_invocation = false;
      allocs.erase(old_ptr);
      time_t t = time(nullptr);
      allocs.insert(pair<void*, pair<size_t, time_t>>(new_ptr, {new_size, t}));
      app_invocation = true;
    }
    print_stats();
  }

 private:
  static const uint32_t kNumSizeDistributionBins =
      12;  // Number of bins for allocation size histogram
  static const uint32_t kNumAgeDistributionBins =
      5;  // Number of bins for allocation age histogram
  static const uint16_t kBarLength = 20;  // In '#' units
  static const uint16_t print_periodicity_secs = 5;

  time_t last_print_time = 0;

  map<void*, pair<size_t, time_t>> allocs;
  uint32_t overall_allocations = 0;

  // Flag to track calls by instrumentor
  volatile bool app_invocation = true;

  void human_readable_bytes(uint32_t num_bytes, string* suffix, float* count) {
    static string suffixes[6] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
    *count = num_bytes;
    uint32_t suffix_idx = 0;
    while (*count >= 1024 && suffix_idx < 6) {
      *count /= 1024;
      suffix_idx++;
    }
    *suffix = suffixes[suffix_idx];
  }

  void print_stats() {
    time_t now = time(nullptr);

    double diff = difftime(now, last_print_time);
    if (diff < print_periodicity_secs) {
      return;
    }
    last_print_time = now;

#define TIME_STR_LEN 100
    char time_str[TIME_STR_LEN];
    time_t rawtime = time(nullptr);
    struct tm* timeinfo = localtime(&rawtime);

    setlocale(LC_NUMERIC, "");

    strftime(time_str, TIME_STR_LEN, "%c", timeinfo);
#undef TIME_STR_LEN

    fprintf(stderr, "\n>>>>>>>>>>>>>%s>>>>>>>>>>>>>\n", time_str);

    // Print number of current allocations
    size_t num_curr_allocs = allocs.size();
    fprintf(stderr, "%'lu Current allocations\n", num_curr_allocs);

    // Print number of overall allocations since starts
    fprintf(stderr, "%'u Overall allocations since start\n",
            this->overall_allocations);

    // Iterate through current allocations and count size of current total
    // allocations, distribution by size and age

    uint64_t num_bytes = 0;
    uint64_t size_distribution[kNumSizeDistributionBins] = {0};
    uint64_t time_distribution[kNumSizeDistributionBins] = {0};

    for (auto const& x : allocs) {
      pair<size_t, time_t> size_and_time = x.second;
      size_t size = size_and_time.first;
      time_t alloc_time = size_and_time.second;

      num_bytes += size;

      // Distribution by size
      size_t i = 0;
      uint32_t current_bin = 4;
      while (i < kNumSizeDistributionBins && size > current_bin) {
        current_bin *= 2;
        i++;
      }
      size_distribution[i]++;

      // Distribution by time
      double seconds = difftime(now, alloc_time);
      i = 0;
      current_bin = 1;
      while (i < kNumAgeDistributionBins && seconds > current_bin) {
        current_bin *= 10;
        i++;
      }
      time_distribution[i]++;
    }

    // Size of current total allocated bytes
    float human_readable_count;
    string suffix;
    human_readable_bytes(num_bytes, &suffix, &human_readable_count);
    fprintf(stderr, "%0.1f %s Current total allocated size\n",
            human_readable_count, suffix.c_str());

    // Display bar chart of distribution of allocations by size
    float unit_allocation = (float)num_curr_allocs / (float)kBarLength;
    fprintf(stderr,
            "\nCurrent allocations by size: ( # = %'.1f current allocations)\n",
            unit_allocation);

    for (uint32_t i = 0, current_bin = 4; i < kNumSizeDistributionBins;
         i++, current_bin *= 2) {
      uint32_t lower_bin = current_bin == 4 ? 0 : current_bin / 2;

      if (i == kNumSizeDistributionBins - 1) {
        fprintf(stderr, "%u + bytes: ", lower_bin);
      } else {
        fprintf(stderr, "%u - %u bytes: ", lower_bin, current_bin);
      }

      uint32_t bar_length =
          (uint32_t)((float)size_distribution[i] / (float)unit_allocation);
      for (uint32_t j = 0; j < bar_length; j++) {
        fprintf(stderr, "#");
      }
      fprintf(stderr, " \n");
    }

    // Display bar chart of distribution of allocations by age
    fprintf(stderr,
            "\nCurrent allocations by age: ( # = %'.1f current allocations)\n",
            unit_allocation);
    for (uint32_t i = 0, current_bin = 1; i < kNumAgeDistributionBins;
         i++, current_bin *= 10) {
      if (i == kNumAgeDistributionBins - 1) {
        fprintf(stderr, "> %u sec: ", current_bin / 10);
      } else {
        fprintf(stderr, "< %u sec: ", current_bin);
      }

      uint32_t bar_length =
          (uint32_t)((float)time_distribution[i] / (float)unit_allocation);
      for (uint32_t j = 0; j < bar_length; j++) {
        fprintf(stderr, "#");
      }
      fprintf(stderr, " \n");
    }
    fflush(stderr);
  }
};

namespace {
orig_malloc_f_type orig_malloc_ = nullptr;
orig_free_f_type orig_free_ = nullptr;
orig_calloc_f_type orig_calloc_ = nullptr;
orig_realloc_f_type orig_realloc_ = nullptr;
Instrumentor instrumentor;
}  // namespace

static bool is_initialized_ = false;

static void initialize(void) {
  if (!is_initialized_) {
    void* handle = RTLD_NEXT;
    orig_malloc_ = (orig_malloc_f_type)dlsym(handle, "malloc");
    orig_free_ = (orig_free_f_type)dlsym(handle, "free");
    orig_calloc_ = (orig_calloc_f_type)dlsym(handle, "calloc");
    orig_realloc_ = (orig_realloc_f_type)dlsym(handle, "realloc");

    instrumentor = Instrumentor();
    is_initialized_ = true;
  }
}

void* malloc(size_t size) {
  initialize();

  void* ptr = orig_malloc_(size);

  if (ptr != nullptr) {
    instrumentor.malloc(ptr, size);
  }

  return ptr;
}

void free(void* ptr) {
  initialize();
  orig_free_(ptr);
  instrumentor.free(ptr);
}

void* realloc(void* ptr, size_t size) {
  initialize();
  void* new_ptr = orig_realloc_(ptr, size);
  instrumentor.realloc(ptr, new_ptr, size);
  return new_ptr;
}

// dlsym calls calloc which is needed to acquire original calloc function.
// https://stackoverflow.com/questions/7910666/problems-with-ld-preload-and-calloc-interposition-for-certain-executables
static char calloc_buffer[8192];

void* calloc(size_t num, size_t size) {
  if (orig_calloc_ == nullptr) {
    // In case this call is from dlsym
    if (num * size > 8192) {
      fprintf(stderr, "Could not initialize calloc.");
      exit(1);
    }
    return calloc_buffer;
  }
  initialize();

  void* ptr = orig_calloc_(num, size);
  size_t total_bytes = num * size;
  if (ptr != nullptr) {
    instrumentor.malloc(ptr, total_bytes);
  }
  return ptr;
}