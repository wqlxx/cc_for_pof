#include "mem_pool.h"


#define MAX_MEM_LIST 256
int print_msg = 1;


static mem_list_t mem_list[MAX_MEM_LIST];
static int mem_list_init = 0;

static int
init_all_mem_list(void)
{
	int i = 0;
	for( i = 0; i < MAX_MEM_LIST; i++)
	{
		mem_list[i].tail  = NULL;
		mem_list[i].head  = NULL;
		mem_list[i].type  = 0;
		mem_list[i].valid = 0;
	}

	return 1;
}

uint8_t *
new_mem_node(uint8_t type)
{
	uint8_t *prt = NULL;
	mem_node_t *plist = mem_list[type].head;
	if(!plist)
		return NULL;

	pthread_spin_lock(&mem_list[type].lock);
	
	if(mem_list[type].tail != mem_list[type].head)
	{
		prt = (uint8_t*)(mem_list[type].tail->data + 1);
		if(print_msg)
			printf("alloc one node:%p\n", prt);
		mem_list[type].tail->data = NULL;
		mem_list[type].tail = mem_list[type].tail->pre;
	}else{
		if(print_msg)
			printf("no node for alloc\n");
	}	

	pthread_spin_unlock(&mem_list[type].lock);
	return prt;
}


void
free_mem_node(uint8_t *addr)
{
	uint8_t type = *(addr - 1);
	mem_list_t *plist = &mem_list[type];
	
	pthread_spinlock(&plist->lock);

	if(plist->tail->next && addr)
	{
		if(print_msg)
			printf("free one mem node %p of type: %u\n", addr,type);
		plist->tail = plist->tail->next;
		plist->tail->data = (uint8_t *)(addr - 1);
	}else{
		if(print_msg)
			printf("free node error\n");
	}	
	
	pthread_spin_unlock(&plist->lock);
	
}


int
init_mem_list(uint8_t type, int size, int length)
{
	int i = 0;
	mem_list_t *plist = &mem_list[i];
	
	if(!mem_list_init)
	{
		init_all_mem_list();
		mem_list_init = 1;
	}

	if((type == 0) || (size <= 0) || (length <= 0))
		return 0;

	plist = &mem_list[type];
	
	if(plist->valid)
		return 1;

	plist->valid = 1;
	pthread_spin_init(&plist->lock, 0);	
	plist->type = type;
	
	for(i = 0; i < length + 1; i++)
	{
		uint8_t *ptype = 0;
		mem_node_t *p = (mem_node_t *)malloc(sizeof(mem_node_t));
		if(!p)
			goto err;
	
		p->data = malloc(size + sizeof(uint8_t));
		if(!p->data)
		{
			free(p);
			goto err;
		}
		p->raw_data = p->data;
		ptype = p->data;
		*ptype = type;
		
		p->next = NULL;
		p->pre = NULL;
		
		if(!plist->head)
		{
			plist->head= p;
			plist->tail = p;
			continue;
		}

		p->next = plist->head->next;
		plist->head->next = p;
		p->pre = plist->head;
		if(p->next)
			p->next->pre = p;
	}
	
	while(plist->tail->next)
		plist->tail = plist->tail->next;
	plist->valid = 1;
	if(print_msg)
		printf("init %d node(s) of type %u success\n", length, type);	
	return 1;

err:
	while(plist->head)
	{
		mem_node_t *p = plist->head;
		plist->head = plist->head->next;
		if(p)
		{
			if(p->data)
				free(p->data);
			free(p);
		}
	}

	mem_list[type].valid = 0;
	mem_list[type].type  = 0;
	mem_list[type].head  = NULL;
	mem_list[type].tail   = NULL;	
	
	return 0;
}


void
clean_mem_list(void)
{
	int num = 0;
	int i = 0;
	uint8_t type;
	
	for(i = 0; i < MAX_MEM_LIST; i++)
	{
		mem_list_t *plist = &mem_list[i];
		int left = -1;
		int free_num = 0;
		if(!plist->valid)
			continue;
		plist->valid = 0;
		type = plist->type;
		num = 0;
		while(plist->head)
		{
			mem_node_t *p = plist->head;
			if(p == plist->tail)
				left = 0;
			
			plist->head = plist->head->next;	
			if(p)
			{
				num++;
				if(left >= 0)
					left++;
				if(p->raw_data)
				{
					free_num++;
					free(p->raw_data);
				}
				free(p);
			}
		}
		if(print_msg)
			printf("free %d mem node(s) of type %u success, %d node(s) being used now, %d free.\n", num, type, left-1, free_num);
	}
}

