#include<allok.h>

#include<stdio.h>
#include<stdint.h>

#include<sys/mman.h>

#include<string.h>
#include<stddef.h>

#include<linkedlist.h>
#include<bst.h>

// doubly linked list to find next and previous memory blocks quickly
static linkedlist blocks_list;

typedef struct block_header block_header;
struct block_header
{
	llnode blocks_node;
	bstnode free_node;
	size_t payload_size;
	char payload[];
};

// block compare comparator
// to compare blocks by size in the free_tree bst
int block_compare(const void* data1, const void* data2)
{
	block_header* h1 = (block_header*) data1;
	block_header* h2 = (block_header*) data2;
	if(h1->payload_size > h2->payload_size)
		return 1;
	else if(h1->payload_size < h2->payload_size)
		return -1;
	else
		return 0;
}

// MAX_BLOCK_SIZE must be kept a multiple of page size of the OS
// on linux this must be 4096

#define PAGE_SIZE          (4096 *  4)
#define PAGE_ALIGN         (PAGE_SIZE)

#define MAX_BLOCK_SIZE     (PAGE_SIZE)
#define MIN_BLOCK_SIZE     (sizeof(block_header) + 8)

#define MAX_PAYLOAD_SIZE   (MAX_BLOCK_SIZE - sizeof(block_header))
#define MIN_PAYLOAD_SIZE   (MIN_BLOCK_SIZE - sizeof(block_header))

// the total_size here includes the size of the block_header struct and the payload size
// i.e. total_size = sizeof(block_header) + payload_size;
static void init_block(void* block, size_t total_size)
{
	block_header* blockH = block;
	initialize_llnode(&(blockH->blocks_node));
	initialize_bstnode(&(blockH->free_node));
	blockH->payload_size = total_size - sizeof(block_header);
}

static block_header* get_new_block()
{
	void* block = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
	init_block(block, PAGE_SIZE);
	insert_head(&blocks_list, block);
	return block;
}

static size_t get_total_block_size(const block_header* blockH)
{
	return sizeof(block_header) + blockH->payload_size;
}

static int is_first_block_of_the_page(const block_header* blockH)
{
	return (((uintptr_t)blockH) % PAGE_ALIGN) == 0;
}

static int is_last_block_of_the_page(const block_header* blockH)
{
	const void* end_of_block = ((const void*)blockH) + get_total_block_size(blockH);
	return (((uintptr_t)end_of_block) % PAGE_ALIGN) == 0;
}

static const block_header* get_next_adjacent_block_of(const block_header* blockH)
{
	// if this block is the last block on the page
	// then, it can not have a next adjacent block
	if(is_last_block_of_the_page(blockH))
		return NULL;

	// find the next block in list
	const block_header* next_blockH = get_next_of(&blocks_list, blockH);

	// check if this block is physically after blockH
	if((((void*)blockH) + get_total_block_size(blockH)) == ((void*)next_blockH))
		return next_blockH;
	return NULL;
}

static const block_header* get_previous_adjacent_block_of(const block_header* blockH)
{
	// if this block is the first block on the page
	// then, it can not have a previous adjacent block
	if(is_first_block_of_the_page(blockH))
		return NULL;

	// find the previous block in list
	const block_header* prev_blockH = get_prev_of(&blocks_list, blockH);

	// check if this block is physically before blockH
	if((((void*)prev_blockH) + get_total_block_size(prev_blockH)) == ((void*)blockH))
		return prev_blockH;
	return NULL;
}

static void delete_used_block(block_header* blockH)
{
	remove_from_linkedlist(&blocks_list, blockH);
	munmap(blockH, PAGE_SIZE);
}

// returns the other splitted block header, if splitted
static block_header* split(block_header* big_blockH, size_t required_payload_size)
{
	if(big_blockH->payload_size < required_payload_size + MIN_BLOCK_SIZE)
		return NULL;

	// total_size of the new block that will be formed after the split
	size_t new_block_total_size = big_blockH->payload_size - required_payload_size;

	// update the payload size of the big_blockH which is now, not so big after all
	big_blockH->payload_size = required_payload_size;

	// find the new address of the split block
	block_header* new_blockH = (block_header*)(big_blockH->payload + big_blockH->payload_size);
	init_block(new_blockH, new_block_total_size);

	// insert the new block in the blocks_list after the big_blockH
	insert_after(&blocks_list, big_blockH, new_blockH);

	return new_blockH;
}

// merges blockH and its blockH->next header if it is not NULL
static int merge(block_header* blockH)
{
	// find the next adjacent block of the blockH,
	// fail the merge operation with a return 0 if it does not exist
	const block_header* next_blockH = get_next_adjacent_block_of(blockH);
	if(next_blockH == NULL)
		return 0;

	// add total block size of the next block to the blockH's payload_size
	blockH->payload_size = blockH->payload_size + get_total_block_size(next_blockH);

	// remove next_block from the linkedlist
	remove_from_linkedlist(&blocks_list, next_blockH);

	return 1;
}

// binary search tree to link free nodes, so we can find best fit free nodes in O(log(n))
static bst free_tree;

// returns 1, if a block is free
static int is_free_block(const block_header* blockH)
{
	return (!is_new_bstnode(&free_tree, &(blockH->free_node))) || (free_tree.root == &(blockH->free_node));
}

void allok_init(int debugL)
{
	initialize_linkedlist(&blocks_list, offsetof(block_header, blocks_node));
	initialize_bst(&free_tree, RED_BLACK_TREE, offsetof(block_header, free_node), block_compare);
}

void* allok(size_t size)
{
	if(size < MIN_PAYLOAD_SIZE)
		size = MIN_PAYLOAD_SIZE;

	if(size > MAX_PAYLOAD_SIZE)
		return NULL;

	block_header* blockH = (block_header*) find_succeeding_or_equals(&free_tree, &((block_header){.payload_size = size}));
	if(blockH == NULL)
		blockH = get_new_block();
	else
		remove_from_bst(&free_tree, blockH);

	if(blockH->payload_size > size)
	{
		block_header* splitted_block = split(blockH, size);
		if(splitted_block != NULL)
			insert_in_bst(&free_tree, splitted_block);
	}

	return blockH->payload;
}

void freek(void* mptr)
{
	if(mptr == NULL)
		return;

	block_header* blockH = (mptr - sizeof(block_header));

	if(is_free_block(blockH))
		return;

	// perform merge of blockH with its next and previous adjacent blocks
	// blockH must hold the address of the largest block formed after merging
	{
		// try to merge blockH with its next adjacent block
		block_header* next_block = (block_header*) get_next_adjacent_block_of(blockH);
		if(next_block != NULL && is_free_block(next_block) && merge(blockH))
			remove_from_bst(&free_tree, next_block);

		// try to merge blockH with its previous adjacent block
		block_header* prev_block = (block_header*) get_previous_adjacent_block_of(blockH);
		if(prev_block != NULL && is_free_block(prev_block) && merge(prev_block))
		{
			remove_from_bst(&free_tree, prev_block);
			blockH = prev_block;
		}
	}
	
	if(blockH->payload_size == MAX_PAYLOAD_SIZE)
		delete_used_block(blockH);
	else
		insert_in_bst(&free_tree, blockH);
}


/*
**
**	DEBUG PRINT FUNCTIONS
**
*/

static void debug_print_block(const block_header* blockH)
{
	if(is_first_block_of_the_page(blockH))
		printf("\tSTART OF PAGE\n\n");

	printf("\t\tBlock :\n");
	printf("\t\t\tAddress : %p\n", blockH);
	printf("\t\t\tIs free : %d\n", is_free_block(blockH));
	printf("\t\t\tPayload : %p\n", blockH->payload);
	printf("\t\t\tSize    : %lu  OR  0x%lx\n\n", blockH->payload_size, blockH->payload_size);

	if(is_last_block_of_the_page(blockH))
		printf("\t  -- xx -- xx --\n\n");
}

static void debug_print_block_wrapper(const void* blockH, const void* add_params_NULL)
{
	debug_print_block(blockH);
}

void debug_print_allok()
{
	printf("Block Header size : %lu OR 0x%lx\n\n", sizeof(block_header), sizeof(block_header));
	if(!is_empty_linkedlist(&blocks_list))
		for_each_in_linkedlist(&blocks_list, debug_print_block_wrapper, NULL);
	else
		printf("\nNo pages in the Allok allocator\n\n");
}