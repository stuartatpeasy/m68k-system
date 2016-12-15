/*
    *** DEAD CODE ***


	block allocator notes

	bit like SLAB allocator in linux

	e.g.
		4096-byte block

		4-byte objects within block
			= 1024 objects / block

		use a bitmap to track alloc/not-alloc'ed objs

		1024 / 8 = 128 bytes needed for bitmap
			= 32 objects' worth

		store bitmap at end of block

		separately, store a record of per-block state:
			block ptr
			alloc unit size (e.g. 4 bytes/obj)
			empty/partial/full


		Use this allocator only for objects <= 256 bytes in size
			-> single byte needed for alloc unit size in block state record
				-> "unit size = 0" means 256-byte unit size
			-> single byte for state

		Fall back to regular malloc()ator for larger allocations, or for
		allocations which aren't likely to align well to fixed sizes
		(e.g. arbitrary strings)
*/

enum ba_block_state
{
	ba_block_empty,
	ba_block_partial,
	ba_block_full
};

struct ba_block
{
	void *ptr;
	u8 alloc_unit_size;
	enum ba_block_state state;
};

struct ba_block_directory
{
	u16 nblocks_free;
	struct ba_block[] blocks;
};

#define BA_BLOCK_SIZE	(4096)

void init_ba_block_directory(struct ba_block_directory *bdir)
{
	u16 u;

	bdir->nblocks_free = (BA_BLOCK_SIZE - sizeof(bdir->nblocks_free)) / sizeof(struct ba_block);
	for (u = 0; u < p->nblocks_free; ++u)
	{
		bdir->blocks[u].ptr = NULL;
		bdir->blocks[u].state = ba_block_empty;
	}
}

void init_ba_block(struct ba_block_directory *bdir, const u8 alloc_size)
{

}
