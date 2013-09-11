#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

struct _node_s{
	uint8_t 	*data;
	uint8_t 	*raw_data;
	struct _node_s 	*next;
	struct _node_s 	*pre;
};	
typedef struct _node_s mem_node_t;

struct mem_list_s {
	mem_node_t 		*head;
	mem_node_t 		*tail;
	pthread_spinlock_t 	lock;
	uint8_t 		type;
	uint8_t			valid;
};
typedef struct mem_list_s mem_list_t;

extern uint8_t *new_mem_node(uint8_t type);
extern void free_mem_node(uint8_t *addr);
extern int init_mem_list(uint8_t type, int size, int length);
extern void clean_mem_list(void);

#endif
