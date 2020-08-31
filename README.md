# Libc memory instrumentation

Author: Abhipray Sahoo (abhiprays@gmail.com)

Shared object (instrumentor.so) that can be used to instrument libc malloc, free, realloc and calloc functions.

# Build and usage

`make` will generate three outputs:
- instrumentor.so: Shared object that can be used with LD_PRELOAD eg. `LD_PRELOAD=$PWD/instrumentor.so python3`. Statistics are printed to stderr every 5 seconds.
- test_app. A multi-threaded app for testing used with. `LD_PRELOAD=$PWD/instrumentor.so ./test_app` or `make instrument_app`. The app will run two threads each invoking 10 malloc calls every 5 seconds. Thus, the expected output increments the total allocations by 20 every 5 seconds.
- test_instrumentor. Run  unit-tests. `./test_instrumentor` or `make instrument_test`. See source for unit tests.

Built with GNU gcc/g++ (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0.