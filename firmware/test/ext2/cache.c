/*
	Block caching system

	Part of the as-yet-unnamed MC68010 operating system.


	(c) Stuart Wallace <stuartw@atom.net>, December 2012.
*/


#define BLOCK_CACHE_NWAYS	(2)
#define BLOCK_CACHE_SLOTS	(16)


struct lru_cache
{
	void *cache;		// this isn't right
	u32 tags[BLOCK_CACHE_NWAYS][BLOCK_CACHE_SLOTS];
	u32 history[BLOCK_CACHE_NWAYS][BLOCK_CACHE_SLOTS];

	u32 hits;
	u32 misses;
};


u32 init_block_cache()
{
}


u32 cached_block_read(ku32 block, void * const buffer)
{
	u32 ret, way;
	ku32 tag = block & ~(BLOCK_CACHE_SLOTS - 1);
	ku32 slot = block & (BLOCK_CACHE_SLOTS - 1);

	for(way = 0; (tags[way][slot] != tag) && (way < BLOCK_CACHE_NWAYS); ++way)
		;	/* empty */

	if(way < BLOCK_CACHE_NWAYS)
	{
		++history[way][slot];
		++hits;
	}
	else
	{
		/* TODO: find least recently used slot */

		ret = block_read(block, cache[way][slot]);
		if(ret)
			return ret;

		history[way][slot] = 0;
		++misses;
	}

	memcpy(buffer, cache[way][slot], BLOCK_SIZE);

	return SUCCESS;
}


u32 cached_block_write(ku32 block, void * const buffer)
{
}

