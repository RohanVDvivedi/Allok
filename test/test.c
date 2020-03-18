#include<mem_man.h>

#include<stdio.h>
#include<stdlib.h>

// We use malloc/free for initialize and test our algorithm Haha irony

#define MEMORY_SIZE 4 * 1024

int main()
{
	void* memory = malloc(MEMORY_SIZE); 

	mem_man* mm = mem_man_init(memory, MEMORY_SIZE);

	

	free(memory);
}