#include <sys/types.h>
#include <limits.h>

#include "run.h"
#include "util.h"

void *base = 0;
void *end = 0;

p_meta find_meta(p_meta *last, size_t size) {
  p_meta index = base;
  p_meta result = base;

  if(base == last)
	  return -1;

  switch(fit_flag){
    case FIRST_FIT:
    {
      //FIRST FIT CODE
	    while(index)
	    {
		    if(result != base)
			    break;
		    if((index->free && index->size >= size) || (result->size > index->size) || !index->next)
			    result = index;

		    index = index->next;
	    }
    }
    break;

    case BEST_FIT:
    {
      //BEST_FIT CODE
	    while(index)
	    {
		    if(result != base)
			    break;
		    if((index->free && index->size >= size && result->size > index->size) || !index->next)
		    	    result = index;
		    
		    index = index->next;
	    }
    }
    break;

    case WORST_FIT:
    {
      //WORST_FIT CODE
	    while(index)
	    {
		    if(result != base)
			    break;
		    if((index->free && index->size >= size && result->size < index->size) || !index->next)
			    result = index;

		    index = index->next;
	    }
    }
    break;

  }
  return result;
}

void *m_malloc(size_t size) {
//	printf("malloc start\n");

	if(base == 0)
	{
		base = sbrk(0);
		end = base;
	}
	
	if(size % 4)
		size += (4 - (size%4));

	int l = size + META_SIZE;

	p_meta t1 = find_meta(end, size);

	p_meta t2 = end;
	
	end += l;

	if(brk(end) == -1)
		return;
	t2->free = 0;
	t2->next = 0;
	t2->prev = t1;
	t2->size = size;

	if(t1 != -1)
		t1->next = t2;
	t1 = t2;

	return t1->data;

//	printf("malloc done\n");
}

void m_free(void *ptr) {
//	printf("free start\n");
	p_meta t1 = ptr - META_SIZE;

	t1->free = 1;

	if(t1->next && t1->next->free == 1)
	{
		t1->size += t1->next->size + META_SIZE;
		t1->next = t1->next->next;
	}

	if(t1->prev != -1)
	{
		if(t1->prev->free)
		{
			t1 = t1->prev;
			t1->size += t1->next->size + META_SIZE;
			t1->next = t1->next->next;
		}
		if(!t1->next)
		{
			end -= t1->size + META_SIZE;
			t1->prev->next = 0;
		}
	}
	else if(!t1->next && !t1->prev)
	{
		end = base;
	}
	ptr = 0;
//	printf("free done\n");
}

void* m_realloc(void* ptr, size_t size)
{
//	printf("realloc start\n");
	p_meta t = ptr - META_SIZE;
	
	if(size % 4)
		size += (4 - (size%4));

	if(t->size == size)
		return ptr;

	else if(t->size < size)
	{
		if(t->next && t->next->free && t->size + t->next->size + META_SIZE >= size)
		{
			t->size += t->next->size + META_SIZE;
			t->next = t->next->next;
			t->next->prev = t;

			if(t->size - size < META_SIZE)
				return ptr;
			else
			{
				p_meta t1 = (int)t + size + META_SIZE;
				t1->prev = t;
				t1->next = t->next;
				t1->size = t->size - size - META_SIZE;
				t->next = t1;
				t->size = size;
				t->free = 0;
				m_free(t1->data);

				return t->data;
			}
		}
		else
		{
			m_free(t->data);
			void* ptr1 = m_malloc(size);
			strcpy(ptr1, ptr);
			return ptr1;
		}
	}
	else if(t->size - size < META_SIZE)
		return ptr;
	else
	{
		p_meta t1 = (int)t + size + META_SIZE;
		t1->prev = t;
		t1->next = t->next;
		t1->size = t->size - size - META_SIZE;
		t->next = t1;
		t->size = size;
		t->free = 0;

		m_free(t1->data);
		return t->data;
	}
//	printf("realloc done\n");
}
