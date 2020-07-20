# Allok
BST based generic memory allocator replacement for malloc, log(n) best fit algorithm for high memory utilization and speed, but has high memory overhead than stdlib malloc.

Note: the alocator is thread-unsafe, use global locks in your application.

setup instructions

 * git clone https://github.com/RohanVDvivedi/Allok.git
 * cd Allok
 * sudo make clean install
 * add "-lallok" linker flag, while compiling your application