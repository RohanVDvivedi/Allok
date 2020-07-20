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
		memory[i] = allok(memory_sizes[i]);
	}

	for(int i = 0; i < TEST_ALLOKS; i++)
	{
		freek(memory[i]);
	}

	return 0;
}