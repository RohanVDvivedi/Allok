#include<mem_man.h>

#include<bst.h>

// lock to protect the tree of nodes
static pthread_mutex_t lock;

// binary search tree to link free nodes, so we can find best fit free nodes
static bst free_tree;

typedef struct header header;
struct header
{
	size_t total_size;
	bstnode free_node;
}

typedef struct footer footer;
struct footer
{
	int is_free;
	size_t total_size;
}

#define get_header_from_mptr(mptr) (((void*)(mptr)) - sizeof(header))
#define get_mptr_from_header(hedr) (((void*)(hedr)) + sizeof(header))

#define get_footer_from_header(hedr) (((void*)(hedr)) + sizeof(header) + ((header*)(hedr))->size)
#define get_header_from_footer(futr) (((void*)(futr)) - ((footer*)(futr))->size - sizeof(header))

// if you request block smaller than this you will get MIN_BLOCK_SIZE
#define MIN_BLOCK_SIZE  (sizeof(header) + sizeof(footer) + 8)

void allok_init(size_t size)
{
	pthread_mutex_init(&lock, NULL);
	initialize_bst(&free_tree);
}

void init_block(void* block, size_t total_size)
{
	header* blockH = block;
	footer* blockF = block + total_size - sizeof(footer);
	blockH->total_size = total_size;
	initialize_bstnode(&(blockH->free_node));
	blockF->is_free = 1;
	blockF->total_size = total_size;
}

header* split(header* big_blockH, size_t split_at_size)
{
	header* split_blockH = (header*)(((void*)big_blockH) + split_at_size);
	size_t split_block_size = big_blockH->size - split_at_size;

	init_block(big_blockH, split_at_size);
	init_block(split_blockH, split_block_size);

	return split_blockH;
}

void* allok(size_t size)
{
	size_t total_size = sizeof(header) + size + sizeof(footer);

	if(size < MIN_BLOCK_SIZE)
	{
		size = MIN_BLOCK_SIZE;
	}

	pthread_mutex_lock(&lock);

	header* blockH = (header*) find_succeeding_or_equals(&free_tree, &total_size);
	remove_from_bst(&free_tree, blockH);

	if(blockH != NULL)
	{
		if(blockH->total_size > total_size + MIN_BLOCK_SIZE)
		{
			header* block_Splited = split(blockH, total_size);
			insert_in_bst(&free_tree, block_Splited);
		}
		footer* blockF = get_footer_from_header(blockH);
		blockF->is_free = 0;
	}

	pthread_mutex_unlock(&lock);

	// TODO
	return (blockH != NULL) ? get_mptr_from_header(blockH) : NULL;
}

void freek(void* mptr)
{
	header* blockH = get_header_from_mptr(mptr);
	// TODO
}