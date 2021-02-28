#ifndef MEM_MAN_H
#define MEM_MAN_H

#include<stdlib.h>

// this should be the first line of your main function for using this allocator
void allok_init(int debugL);

// allocates memory of given size and returns the pointer pointing to it
void* allok(size_t size);

// frees previously allocated memory block
void freek(void* mptr);

/*
	General use case ::

		void* mptr = allok( SIZE );

		// your logic

		freek(mptr);
*/

#endif