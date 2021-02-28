#include<allok.h>

#include<stdio.h>
#include<stdlib.h>

#define TEST_ALLOKS 300

unsigned int ALLOCATION_RUNS_COUNT = 3;
unsigned int ALLOK_RUNS_SIZES[] = {100, 100, 100};
unsigned int FREEK_RUNS_SIZES[] = { 60,  90, 150};

unsigned int test_sizes_size = 6;
unsigned int test_sizes[] = {8, 16, 20, 40, 64, 96};

int memory_sizes[TEST_ALLOKS];
void* memory[TEST_ALLOKS];

int main()
{
	unsigned int total_allocation_size = 0;

	for(unsigned int i = 0; i < TEST_ALLOKS; i++)
	{
		memory_sizes[i] = test_sizes[ rand() % test_sizes_size ];
		total_allocation_size += memory_sizes[i];
	}

	printf("Total usable memory size: %u\n\n", total_allocation_size);

	allok_init();
	printf("Mememory allocator initialized\n\n");

	debug_print_allok();

	for(unsigned int i = 0; i < TEST_ALLOKS; i++)
	{
		memory[i] = allok(memory_sizes[i]);
		printf("%d - Allocated memory for size %u at %p\n\n", i, memory_sizes[i], memory[i]);
	}

	debug_print_allok();

	for(unsigned int i = 0; i < TEST_ALLOKS; i++)
	{
		freek(memory[i]);
		printf("%d - Freed memory for size %u at %p\n\n", i, memory_sizes[i], memory[i]);
	}

	debug_print_allok();

	return 0;
}