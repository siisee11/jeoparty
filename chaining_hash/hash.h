#ifndef _HASH_H
#define _HASH_H

#include <stdbool.h> 
#include <stdio.h>
#include <stdint.h>
#include "list.h"
#include "rcupdate.h"

struct uint48 {
	    uint64_t x:48;
} __attribute__((packed));

int kmalloc_verbose;
int test_verbose;

#define printv(verbosity_level, fmt, ...) \
    if(test_verbose >= verbosity_level) \
        printf(fmt, ##__VA_ARGS__)


#define BITS_PER_LONG 	(64)
#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])

struct hash_iter {
	unsigned long	index;
	unsigned long	next_index;
	struct hash_node *node;
};

struct head {
	union {
		struct {
			struct uint48 __rcu addr;
			unsigned short counter : 15;  	/* counter */
			unsigned short active : 1; 		/* active bit */
		};
		struct hash_node *node;
	};
} __attribute__((packed));

struct hash {
	unsigned long size;
	void __rcu **slots;
};

struct hash_node {
	void *item;
	struct list_head list;
};

struct item {
   int data;
   int index;
};

/* hash function */
static inline int hashCode(struct hash *h, int key) {
   return key % (h->size);
}

struct hash *hash_alloc(unsigned long size);
int hash_insert(struct hash *, unsigned long index, void *);
int hash_get(struct hash*, unsigned long index,
			  struct hash_node **nodep);
void *hash_lookup(struct hash*, unsigned long index);
void display(struct hash *);

/**
 * hash_iter_init - initialize radix tree iterator
 *
 * @iter:	pointer to iterator state
 * @start:	iteration starting index
 * Returns:	NULL
 */
static __always_inline void **
hash_iter_init(struct hash_iter *iter, unsigned long start)
{
	iter->index = 0;
	iter->next_index = start;
	return NULL;
}

static inline unsigned long
__hash_iter_add(struct hash_iter *iter, unsigned long slots)
{
	return iter->index + slots;
}

static __always_inline void **hash_next_slot(void **slot,
				struct hash_iter *iter)
{
	long count = 20;

	while (--count > 0) {
		slot++;
		iter->index = __hash_iter_add(iter, 1);

		if (*slot)
			goto found;
	}
	return NULL;

 found:
	return slot;
}



/**
 * hash_for_each_slot - iterate over non-empty slots
 *
 * @slot:	the void** variable for pointer to slot
 * @root:	the struct hash pointer
 * @iter:	the struct hash_iter pointer
 * @start:	iteration starting index
 *
 * @slot points to radix tree slot, @iter->index contains its index.
 */
#define hash_for_each_slot(slot, root, iter, start)		\
	for (slot = hash_iter_init(iter, start) ;			\
	     slot || (slot = hash_next_chunk(root, iter, 0)) ;	\
	     slot = hash_next_slot(slot, iter, 0))


#endif
