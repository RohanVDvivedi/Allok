#ifndef MEM_MAN_H
#define MEM_MAN_H

#include<stdint.h>

#define NULL 0

typedef struct mem_man mem_man;
struct mem_man
{
	unsigned long long int memory_size;

	// this is the root of the binary search tree that maintains the free blocks
	void* root;

	// this is the first block in the memory manager
	void* first_block;

	// it is the largest free memory that we can provide to the user
	// if the user happens to ask for a block larger than this size, we return NULL from allok
	unsigned long long int largest_free_blk_size;
};

/*
	structure of a memory block is as below
	{
		mem_man block
	}
	{
		{
			mem_blk_prefix
		}
		{
			actual free memory lies here
		}
		{
			mem_blk_suffix
		}
	}
*/

typedef struct mem_blk_prefix mem_blk_prefix;
struct mem_blk_prefix
{
	// the address of the parent block of this block in balanced binary search tree of free blocks
	void* parent;

	// the address of the left block of this block in balanced binary search tree of free blocks
	void* left;

	// the address of the right block of this block in balanced binary search tree of free blocks
	void* right;

	// size of the block of this memory, it is the size excluding the mem_blk_prefix and mem_blk_suffix size
	// it is the free memory that the user can use or is using
	// negative size indicated that block in not free
	int32_t blk_size;
};

typedef struct mem_blk_suffix mem_blk_suffix;
struct mem_blk_suffix
{
	// size of the block of this memory, it is the size excluding the mem_blk_prefix and mem_blk_suffix size
	// it is the free memory that the user can use or is using
	// negative size indicated that block in not free
	int32_t blk_size;
};

mem_man* mem_man_init(void* memory, uint32_t memory_size);

// to allocate memory of given size on provided memory manager's context
void* allok(mem_man* mem_man_p, uint32_t size);

// to free memory on provided memory manager's context, 
// by giving back the same memory pointer that was returned by allok 
void freek(mem_man* mem_man_p, void* mem);

#endif