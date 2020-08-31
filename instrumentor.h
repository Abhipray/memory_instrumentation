#include <stdlib.h>

#include <cstring>
#include <map>
#include <mutex>

using namespace std;

class Instrumentor {
 public:
  /**
   * @brief This should be called when a malloc is requested by the app
   */
  void malloc(void* ptr, size_t size);

  /**
   * @brief This should be called when a malloc is requested by the app
   */
  void free(void* ptr);

  /**
   * @brief This should be called when a realloc is requested by the app
   */
  void realloc(void* old_ptr, void* new_ptr, size_t new_size);

  /** Expose methods useful for testing class */
  void get_alloc_map(map<void*, pair<size_t, time_t>>* allocs);
  void freeze();
  void unfreeze();

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
  recursive_mutex class_mutex;

  // Flag to track calls by instrumentor
  volatile bool app_invocation = true;

  /**
   * @brief Convert byte count into human readable format with a suffix and a
   * floating point prefix.
   */
  void human_readable_bytes(uint32_t num_bytes, string* suffix, float* count);

  void print_stats();
};

/**
 * @brief Allocate memory block
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.
 *
 * The content of the newly allocated block of memory is not initialized,
 * remaining with indeterminate values.
 *
 * If size is zero, the return value depends on the particular library
 * implementation (it may or may not be a null pointer), but the returned
 * pointer shall not be dereferenced.
 * @param size Size of the memory block, in bytes.
 * size_t is an unsigned integral type.
 * @return void* On success, a pointer to the memory block allocated by the
 * function. The type of this pointer is always void*, which can be cast to the
 * desired type of data pointer in order to be dereferenceable. If the function
 * failed to allocate the requested block of memory, a null pointer is returned.
 */
extern "C" void* malloc(size_t size);
typedef void* (*orig_malloc_f_type)(size_t size);

/**
 * @brief Deallocate memory block
 * A block of memory previously allocated by a call to malloc, calloc or realloc
 * is deallocated, making it available again for further allocations.
 *
 * If ptr does not point to a block of memory allocated with the above
 * functions, it causes undefined behavior.
 *
 * If ptr is a null pointer, the function does nothing.
 *
 * Notice that this function does not change the value of ptr itself, hence it
 * still points to the same (now invalid) location.
 *
 * @param ptr Pointer to a memory block previously allocated with malloc, calloc
 * or realloc.
 */
extern "C" void free(void* ptr);
typedef void (*orig_free_f_type)(void* ptr);

/**
 * @brief Allocate and zero-initialize array
 *
 * Allocates a block of memory for an array of num elements, each of them
 * size bytes long, and initializes all its bits to zero.
 *
 * The effective result is the allocation of a zero-initialized memory block of
 * (num*size) bytes.
 *
 * If size is zero, the return value depends on the particular library
 * implementation (it may or may not be a null pointer), but the returned
 * pointer  * shall not be dereferenced.
 *
 * @param num Number of elements to allocate.
 * @param size Size of each element.
 * @return void*
 */
extern "C" void* calloc(size_t num, size_t size);
typedef void* (*orig_calloc_f_type)(size_t num, size_t size);

/**
 * @brief Reallocate memory block
 * Changes the size of the memory block pointed to by ptr.
 *
 * The function may move the memory block to a new location (whose address is
 * returned by the function).
 *
 * The content of the memory block is preserved up to the lesser of the new and
 * old sizes, even if the block is moved to a new location. If the new size * is
 * larger, the value of the newly allocated portion is indeterminate.
 *
 * In case that ptr is a null pointer, the function behaves like malloc,
 * assigning a new block of size bytes and returning a pointer to its beginning.
 *
 * @param ptr
 * @param size
 * @return void*
 */
extern "C" void* realloc(void* ptr, size_t size);
typedef void* (*orig_realloc_f_type)(void* ptr, size_t size);

void get_alloc_map(map<void*, pair<size_t, time_t>>* allocs);
void freeze_instrumentor();
void unfreeze_instrumentor();