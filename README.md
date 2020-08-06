# Allok
BST based generic memory allocator replacement for malloc, log(n) best fit algorithm for high memory utilization and speed, but has high memory overhead than stdlib malloc.

Note: the alocator is thread-unsafe, use global locks in your application.

 * Note: The allocator uses a lot of memory for management. Just for the idea see the analysis below:
	* After running the test case on my machine with allocation sample sizes {6, 16, 20, 35, 55}.
	  * total usable memory (acquired over 197 allocations) = 5264 bytes
	  * total memory used by the allocator = 4 * 4096 bytes (each page used by the allocator is 16 KB)
	  * total memory used by the allocator for memory management = 56 * 175 bytes ==> 9800 bytes
	  * 32.128 % efficiency in memory usage
 * The memory allocator is not very efficient with smaller memory allocations like 6 - 24 bytes, because each allocated block will use up 56 bytes for its own management internally to the allocator.
 * Use this allocator should be used for larger block sizes for better efficiency.

setup instructions

 * git clone https://github.com/RohanVDvivedi/Allok.git
 * cd Allok
 * sudo make clean install
 * add "-lallok -lcutlery" linker flag, while compiling your application