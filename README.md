# CSCI-442-Project-3-Parallel-Zip
##### using C

## Utilized the POSIX-threads library to employ multiple threads, which accelerates computational tasks 

#### project： implement a parallel zip (pzip) program, using the C programming language andPOSIX-threads. The pzip will read an input text file, which is composed of lowercase letters (a-z) only. As an output it will produce a binary file indicating the consecutive uses of each character. The pzip operation needs to be done in parallel using pthreads library.
##### （1）Call pthread_create() to launch parallel threads.
##### （2）Call pthread_barrier() to synchronize the pthreads, without destroying them.
##### （3）Call pthread_join() to finish the parallel execution and synchronize the pthreads again.
