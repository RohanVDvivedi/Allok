#include<allok.h>

#include<stdio.h>

#include<sys/mman.h>

#include<string.h>
#include<stddef.h>

#include<linkedlist.h>
#include<bst.h>

static int debug = 0;

// binary search tree to link free nodes, so we can find best fit free nodes
static bst free_tree;

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

#define MAX_BLOCK_SIZE (4096 * 4)
#define MIN_BLOCK_SIZE (sizeof(block_header) + 8)

#define MAX_PAYLOAD_SIZE MAX_BLOCK_SIZE - sizeof(block_header)
#define MIN_PAYLOAD_SIZE MIN_BLOCK_SIZE - sizeof(block_header)

void allok_init(int debugL)
{
	debug = debugL;
	if(debug)
		printf("sizeof Block Header : decimal : %lu, in hex : %lx\n\n", sizeof(block_header), sizeof(block_header));
	initialize_linkedlist(&blocks_list, offsetof(block_header, blocks_node));
	initialize_bst(&free_tree, RED_BLACK_TREE, offsetof(block_header, free_node), block_compare);
}

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
	void* block = mmap(NULL, MAX_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
	if(debug)
		printf("->-> New allocation of size : %u\n", MAX_BLOCK_SIZE);
	init_block(block, MAX_BLOCK_SIZE);
	insert_head(&blocks_list, block);
	return block;
}

// returns 1, if a block is free
static int is_free_block(const block_header* blockH)
{
	return (!is_new_bstnode(&free_tree, &(blockH->free_node))) || (free_tree.root == &(blockH->free_node));
}

static void delete_used_block(block_header* blockH)
{
	remove_from_linkedlist(&blocks_list, blockH);
	munmap(blockH, MAX_BLOCK_SIZE);
}

// returns the other splitted block header, if splitted
static block_header* split(block_header* big_blockH, size_t required_payload_size)
{
	if(big_blockH->payload_size < required_payload_size + MIN_BLOCK_SIZE)
		return NULL;

	// total_size of the new block that will be formed
	size_t new_block_total_size = big_blockH->payload_size - required_payload_size;

	// update the payload size of the big_blockH which is now, not so big after all
	big_blockH->payload_size = required_payload_size;

	// find the new address of the split block
	block_header* new_block = (block_header*)(big_blockH->payload + big_blockH->payload_size);
	init_block(new_block, new_block_total_size);

	// insert the new block (the ) in the 
	insert_after(&blocks_list, big_blockH, new_block);

	return new_block;
}

// merges blockH and its blockH->next header if it is not NULL
static int merge(block_header* blockH)
{
	// find the next block of the blockH
	block_header* next_block = (block_header*) get_next_of(&blocks_list, blockH);

	// single block in the linkedlist
	// OR if the block_next is not physically adjacent to the block_next
	// OR if the blockH's payload_size is equal to MAX_PAYLOAD_SIZE
	// then => no merging possible
	if( (next_block == blockH) || (blockH->payload + blockH->payload_size != ((char*)next_block))
		|| (blockH->payload_size == MAX_PAYLOAD_SIZE) )
		return 0;

	// add block_header size and payload size to the blockH's payload_size
	blockH->payload_size = blockH->payload_size + (sizeof(block_header) + next_block->payload_size);

	// remove next_block from the linkedlist
	remove_from_linkedlist(&blocks_list, next_block);

	return 1;
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
	block_header* blockH = (mptr - sizeof(block_header));

	if(is_free_block(blockH))
		return;

	if(blockH->payload_size < MAX_PAYLOAD_SIZE)
	{
		block_header* next_block = (block_header*) get_next_of(&blocks_list, blockH);
		if(next_block != NULL && is_free_block(next_block) && merge(blockH))
			remove_from_bst(&free_tree, next_block);

		block_header* prev_block = (block_header*) get_prev_of(&blocks_list, blockH);
		if(prev_block != NULL && is_free_block(prev_block) && merge(prev_block))
		{
			remove_from_bst(&free_tree, prev_block);
			blockH = prev_block;
		}

		insert_in_bst(&free_tree, blockH);
	}
	
	if(blockH->payload_size == MAX_PAYLOAD_SIZE)
		delete_used_block(blockH);
}