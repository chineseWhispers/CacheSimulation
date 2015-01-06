//Yingyan Wang -- Assignment 6 -- ECE 3056

#ifndef __CACHESIM_H
#define __CACHESIM_H
typedef unsigned long long addr_t;
typedef unsigned long long counter_t;

typedef struct stack{
	int size;
	int array[100];
}stack;

typedef struct set{
	int size;
	int tag[100];
	int valid[100];
	int dirty[100];
}set;

typedef struct cache{
	int assoc;
	int block_size;
	int size;
	set* set;
	stack* stack;
}cache;

void cachesim_init(int blocksize, int cachesize, int ways, int blocksize2, int cachesize2, int ways2);
void cachesim_access(addr_t physical_add, int write);
void stack_init(stack* st);
void cachesim_print_stats(void);
void printStack(stack* st);
int push(stack* st, int value, int remove_index);
void printSet(set* set);
void set_init(set* set, stack* st);
void set_update(set* set, stack* st, int write, int value, int removed_value);
int isHit(stack* st, set* s, int value);
void cache_init(cache* ca, int set_num, int blocksize, int size, int ways);
void printCache(cache* ca, int set_num, int ways);
int cache_update(cache* ca, int remove_index, int value, int write, int set_order);
int isWriteback(stack* st, set* s, int removed_index);

#endif