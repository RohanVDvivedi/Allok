#include<allok.h>

#include<bst.h>

// binary search tree to link free nodes, so we can find best fit free nodes
static bst free_tree;

typedef struct block_header block_header;
struct block_header
{
	block_header* next;
	block_header* prev;
	bstnode free_node;
	size_t payload_size;
	char payload[];
};

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

#define MAX_BLOCK_SIZE (512)//(4096 * 2)
#define MIN_BLOCK_SIZE (sizeof(block_header) + 8)

#define MAX_PAYLOAD_SIZE MAX_BLOCK_SIZE - sizeof(block_header)
#define MIN_PAYLOAD_SIZE MIN_BLOCK_SIZE - sizeof(block_header)

void allok_init()
{
	initialize_bst(&free_tree, RED_BLACK_TREE, offsetof(block_header, free_node), block_compare);
}

void init_block(void* block, size_t total_size)
{
	block_header* blockH = block;
	blockH->next = NULL;
	blockH->prev = NULL;
	initialize_bstnode(&(blockH->free_node));
	blockH->payload_size = total_size - sizeof(block_header);
}

block_header* get_new_block()
{
	printf("Getting new block\n");
	void* block = malloc(MAX_BLOCK_SIZE);
	init_block(block, MAX_BLOCK_SIZE);
	return block;
}

void return_block_memory(block_header* blockH)
{
	free(blockH);
}

int is_free_block(block_header* blockH)
{
	return exists_in_bst(&free_tree, blockH);
}

// returns the other splitted block header, if splitted
block_header* split(block_header* big_blockH, size_t required_payload_size)
{
	printf("block being splitted, has payload_size %lu + block base size %lu\n", big_blockH->payload_size, sizeof(block_header));
	if(big_blockH->payload_size < required_payload_size + MIN_BLOCK_SIZE)
		return NULL;

	size_t new_block_size = big_blockH->payload_size - required_payload_size;
	big_blockH->payload_size = required_payload_size;

	block_header* new_block = (block_header*)(big_blockH->payload + big_blockH->payload_size);
	init_block(new_block, new_block_size);

	new_block->prev = big_blockH;
	new_block->next = big_blockH->next;

	if(big_blockH->next != NULL)
		big_blockH->next->prev = new_block;
	big_blockH->next = new_block;

	return new_block;
}

// merges blockH and its blockH->next header if it is not NULL
int merge(block_header* blockH)
{
	if(blockH->next == NULL)
		return 0;

	blockH->payload_size = blockH->payload_size + sizeof(block_header) + blockH->next->payload_size;

	blockH->next = blockH->next->next;
	if(blockH->next != NULL)
		blockH->next->prev = blockH;

	return 1;
}

void* allok(size_t size)
{
	// cap to MIN_PAYLOAD_SIZE
	if(size < MIN_PAYLOAD_SIZE)
		size = MIN_PAYLOAD_SIZE;

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

	// if it is already free return the block
	if(is_free_block(blockH))
		return;

	if(blockH->next != NULL && is_free_block(blockH->next))
	{
		remove_from_bst(&free_tree, blockH->next);
		merge(blockH);
	}

	if(blockH->prev != NULL && is_free_block(blockH->prev))
	{
		remove_from_bst(&free_tree, blockH->prev);
		blockH = blockH->prev;
		merge(blockH);
	}

	if(blockH->payload_size == MAX_PAYLOAD_SIZE && blockH->prev == NULL && blockH->next == NULL)
		return_block_memory(blockH);
	else
		insert_in_bst(&free_tree, blockH);
}