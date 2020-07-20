#include<allok.h>

#include<stdio.h>
#include<stdlib.h>

#define TEST_ALLOKS 175

unsigned int test_sizes_size = 5;
unsigned int test_sizes[] = {6, 16, 20, 35, 55};

int memory_sizes[TEST_ALLOKS];
void* memory[TEST_ALLOKS];

int main()
{
	unsigned int total_allocation_size = 0;
	allok_init();

	for(unsigned int i = 0; i < TEST_ALLOKS; i++)
	{
		memory_sizes[i] = test_sizes[ rand() % test_sizes_size ];
		printf("%d - Allocating memory for size %u\n", i, memory_sizes[i]);
		memory[i] = allok(memory_sizes[i]);
		printf("%d - Allocated memory for size %u at %p\n\n", i, memory_sizes[i], memory[i]);

		total_allocation_size += memory_sizes[i];
	}

	printf("Allocations Completed\n");

	printf("Total usable memory size: %u\n", total_allocation_size);

	for(unsigned int i = 0; i < TEST_ALLOKS; i++)
	{
		printf("%d - Freeing memory for size %u at %p\n\n", i, memory_sizes[i], memory[i]);
		freek(memory[i]);
	}

	return 0;
}