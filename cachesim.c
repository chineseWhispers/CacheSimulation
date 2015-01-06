//Yingyan Wang -- Assignment 6 -- ECE 3056

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cachesim.h"
//Global variables
counter_t accesses = 0, hits = 0, misses = 0, writebacks = 0, hits2 = 0, misses2 = 0, accesses2 = 0,write_number=0, write_misses=0;
cache* cake;
cache* cupcake;
int block_value, block_value2;
int set_order, set_order2;
int tag_value, tag_value2;
int block_length, block_length2;
int set_length, set_length2;
int tag_length, tag_length2;

//This is the function that initializes the stack. 
//The stack is initially filled with -1.
void stack_init(stack* st,int ways){
	st->size = ways;
	for (int i = 0; i < ways; i++){
		st->array[i] = -1;
	}
}

//This is the function that initializes the set
//It copies the values from the stack, initializes the dirty bit and valid bit to zero.
void set_init(set* set, stack* st){
	set->size = st->size;
	for (int i = 0; i < st->size; i++){
		set->tag[i] = st->array[i];
		set->dirty[i] = 0;
		set->valid[i] = 0;
	}
}

//This is the function that initializes the cache
void cache_init(cache* ca, int set_num, int ways,int blocksize, int size){
	ca->assoc = ways;
	ca->block_size = blocksize;
	ca->size = size;
	set* s = (set*)malloc(sizeof(set)*set_num);
	stack* st = (stack*)malloc(sizeof(stack)*set_num);
	for (int i = 0; i < set_num; i++){
		stack_init(&(st[i]), ways);
		set_init(&s[i], &st[i]);
		ca->set = s;
		ca->stack = st;
	}
}

//This is the function that updates the set. 
//A set is updated when there is a miss.
void set_update(set* set, stack* st, int write,int value,int removed_value){
	for (int i = 0; i < set->size; i++){
		if (set->tag[i] == removed_value){
			set->tag[i] = value;
			set->valid[i] = 1;
			if (write == 1){
				set->dirty[i] = 1;
			}
		}
	}
}

//This is the function that updates the cache.
//The push function is called to update each stack in the cache.
//The set update function is used to update each set in the cache. 
int cache_update(cache* ca, int remove_index, int value, int write,int set_order){
	int removed_value;
	int write_back_indicator;
	removed_value = push(&(ca->stack)[set_order], value, remove_index);
	write_back_indicator=isWriteback(&ca->stack[set_order], &ca->set[set_order], removed_value);
	set_update(&(ca->set[set_order]), &(ca->stack[set_order]), write, value, removed_value);
	return write_back_indicator;
}

//Debugging Functions
void printStack(stack* st){
	for (int i = 0; i < st->size; i++){
		printf("At stack index %d ", i);
		printf("The value is %d \n", st->array[i]);
	}
}

void printSet(set* set){
	for (int i = 0; i < set->size; i++){
		printf("Within the set, at index %d ", i);
		printf("%d ,", set->tag[i]);
		printf("%d ,", set->valid[i]);
		printf("%d \n", set->dirty[i]);
	}
}

void printCache(cache* ca, int set_num, int ways){
	int i = set_num;
	for (int i = 0; i < set_num; i++){
			printf("For Set %d :\n",i);
			printStack(&(ca->stack)[i]);
			printSet(&(ca->set[i]));
	}
}

//This is the function that implements the LRU stack. 
//The value from the physical address is added to the top of the stack.
//This function is also able to remove the value from a specific index.
//When a value is removed from the stack. Every element above it is shifted down by one element. 
int push(stack* st,int value,int remove_index){
	int removed_value;
	removed_value = st->array[remove_index];
	for (int i = 0; i < st->size; i++){
		if (i >= remove_index){
			st->array[i] = st->array[i + 1];
		}
	}
	st->array[st->size - 1] = value;
	return removed_value;
}

// if isHit function returns an index of -1, it's a miss.
// if isHit function returns an index that is not -1, it's a hit
int isHit(stack* st, set* s, int value){
	int hitIndex = -1;
	int checkHit = 0;
	for (int i = 0; i < st->size; i++){
		if (s->tag[i] == value && s->valid[i] == 1){
			checkHit = 1;
		}
	}
	for (int j = 0; j < st->size; j++){
		if (st->array[j] == value){
			hitIndex = j;
		}
	}
	return hitIndex;
}

// This is the function to determine whether writeback occurs.
int isWriteback(stack* st, set* s, int removed_value){
	int writebackcheck = 0;
	int remove_set_index=-1;
	for (int i = 0; i < s->size; i++){
		if (s->tag[i] == removed_value){
			remove_set_index = i;
		}
	}
	if (s->dirty[remove_set_index] == 1){
		writebackcheck = 1;
	}
	return writebackcheck;
}

// This is where the length of different fields. It also takes in the cache_init function.
void cachesim_init(int blocksize, int cachesize, int ways, int blocksize2, int cachesize2, int ways2) {
	block_length = log(blocksize) / log(2);
	set_length = log(cachesize / (blocksize*ways)) / log(2);
	tag_length = 32 - block_length - set_length;
	cake = (cache*)malloc(sizeof(cache));
	cache_init(cake, cachesize / blocksize / ways, ways, blocksize, cachesize);
	block_length2 = log(blocksize2) / log(2);
	set_length2 = log(cachesize2 / (blocksize2*ways2)) / log(2);
	tag_length2 = 32 - block_length2 - set_length2;
	cupcake = (cache*)malloc(sizeof(cache));
	cache_init(cupcake, cachesize2 / blocksize2 / ways2, ways2, blocksize2, cachesize2);
}

//This is where the actual accesses of cache takes place
void cachesim_access(addr_t physical_addr, int write) {
	if (write == 1){
		write_number++;
	}
	int set_num = cake->size / cake->block_size / cake->assoc;
	int set_num2 = cupcake->size / cupcake->block_size / cupcake->assoc;
	int hitIndex;
	int hitIndex2;
	accesses++;
	//Parse the values from the physical address
	block_value = physical_addr & ((1 << block_length) - 1);
	set_order = (physical_addr >> block_length)&((1 << set_length) - 1);
	tag_value = (physical_addr >> (block_length + set_length))&((1 << tag_length) - 1);
	block_value2 = physical_addr & ((1 << block_length2) - 1);
	set_order2 = (physical_addr >> block_length2)&((1 << set_length2) - 1);
	tag_value2 = (physical_addr >> (block_length2 + set_length2))&((1 << tag_length2) - 1);
	hitIndex = isHit(&(cake->stack[set_order]), &(cake->set[set_order]),tag_value);
	hitIndex2 = isHit(&(cupcake->stack[set_order2]), &(cupcake->set[set_order2]), tag_value2);
	//When both caches misses, increase miss of both caches
	if (hitIndex==-1 && hitIndex2 == -1){
		if (cache_update(cake, 0, tag_value, write, set_order)){
			writebacks++;
		}
		cache_update(cupcake, 0, tag_value2, write, set_order2);
		misses++;
		if (write == 1){
			write_misses++;
		}
		accesses2++;
		misses2++;
	}
	//When the first cache misses, and the second cache hits, increase miss for L1 cache
	//Also increase hit for L2 cache
	else if (hitIndex == -1 && hitIndex2 != -1){
		if (cache_update(cake, 0, tag_value, write, set_order)){
			writebacks++;
		}
		cache_update(cupcake, hitIndex2, tag_value2, write, set_order2);
		accesses2++;
		misses++;
		if (write == 1){
			write_misses++;
		}
		hits2++;
	}
    //When the first cache hits, update hits of the first one
	else if (hitIndex !=-1){
		cache_update(cake, hitIndex, tag_value, write, set_order);
		hits++;
	}
}

void cachesim_print_stats() {
	printf("The total number of write is %d \n", write_number);
	printf("The total number of write misses is %d \n", write_misses);
	printf("%llu, %llu, %llu, %llu,%llu,%llu\n", accesses, hits, misses, writebacks, hits2,misses2);
}