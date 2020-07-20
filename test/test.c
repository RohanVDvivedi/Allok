#include<allok.h>

#include<stdio.h>
#include<stdlib.h>

#define TEST_ALLOKS 50

int test_sizes_size = 5;
int test_sizes[] = {6, 16, 20, 35, 55};

int memory_sizes[TEST_ALLOKS];
void* memory[TEST_ALLOKS];

int main()
{
	allok_init();

	for(int i = 0; i < TEST_ALLOKS; i++)
	{
		memory_sizes[i] = test_sizes[ rand() % test_sizes_size ];
		//printf("Allocating memory for size %u\n", memory_sizes[i]);
		memory[i] = allok(memory_sizes[i]);
		//printf("Allocated memory for size %u at %p\n\n", memory_sizes[i], memory[i]);
	}

	//printf("Allocations Completed\n");

	for(int i = 0; i < TEST_ALLOKS; i++)
	{
		//printf("Freeing memory for size %u at %p\n\n", memory_sizes[i], memory[i]);
		freek(memory[i]);
	}

	return 0;
}