# Allok
Binary Search Tree based generic memory allocator replacement for malloc, log(n) best fit algorithm for high memory utilization and speed, but has high memory overhead than stdlib malloc.

Note: the alocator is thread-unsafe, so use global locks in your application, while you allocate and free memory using this allocator.

Note: This allocator does not gaurantee/satisfy any alignment requirements (neither for pointers for internal data structures, nor for returned blocks), hence do not use it on machines with alignments constraints (like ARM or similar RISC machines).

 * Note: The allocator uses a lot of memory for management. Just for the idea see the analysis below:
	* After running the test case on my machine with allocation sample sizes {6, 16, 20, 35, 55}.
	  * total usable memory (acquired over 197 allocations) = 5264 bytes
	  * total memory used by the allocator = 4 * 4096 bytes (each page used by the allocator is 16 KB)
	  * total memory used by the allocator for memory management = 56 * 175 bytes ==> 9800 bytes
	  * 32.128 % efficiency in memory usage
 * The memory allocator is not very efficient with smaller memory allocations like 6 - 24 bytes, because each allocated block will use up 56 bytes for its own management internally to the allocator.
 * This allocator should be used for allocating larger block sizes for better memory usage efficiency, which is gauranteed because of O(log(n)) best fit algorithm.

If you are in-search for an alignment aware best-fit (log(n) search) allocator for preassigned memory block for microcontrollers, then Cutlery's uc_allocator_interface.h might just fit your needs.

## Setup instructions
**Install dependencies :**
 * [Cutlery](https://github.com/RohanVDvivedi/Cutlery)

**Download source code :**
 * `git clone https://github.com/RohanVDvivedi/Allok.git`

**Build from source :**
 * `cd Allok`
 * `make clean all`

**Install from the build :**
 * `sudo make install`
 * ***Once you have installed from source, you may discard the build by*** `make clean`

## Using The library
 * add `-lallok -lcutlery` linker flag, while compiling your application
 * do not forget to include appropriate public api headers as and when needed. this includes
   * `#include<allok/allok.h>`

## Instructions for uninstalling library

**Uninstall :**
 * `cd Allok`
 * `sudo make uninstall`
