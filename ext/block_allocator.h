#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H

class block_allocator
{
private:
	struct block
	{
		std::size_t size;
		std::size_t used;
		char *buffer;
		block *next;
	};

	block *m_head;
	std::size_t m_blocksize;

	block_allocator(const block_allocator &);
	block_allocator &operator=(block_allocator &);

public:
	block_allocator(std::size_t blocksize);
	~block_allocator();

	// exchange contents with rhs
	void swap(block_allocator &rhs);

	// allocate memory
	void *malloc(std::size_t size);

	// free all allocated blocks
	void free();
};

#endif
