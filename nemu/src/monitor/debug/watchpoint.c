#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp()
{
    WP* ret;
    if(free_==NULL)
    {
	printf("The watchpoints have all been occupied\n");
	ret=NULL;
    }
    else
    {
	    ret=free_;
	    free_=free_->next;
        ret->next=NULL;
        if(head==NULL)
        {
            head=ret;
            ret->NO=1;
        }
        else
        {
            WP *tmp=head;
            int i=1;
            while(tmp->next)
            {
                tmp->NO=i;
                tmp=tmp->next;
                ++i;
            }
            tmp->NO=i;
            i++;
            ret->NO=i;
            tmp->next=ret;
        }
    }
    return ret;
}

void free_wp(WP *wp_to_delete)
{
    WP* tmp=head;
    int i;
    if(tmp==wp_to_delete)
    {
        head=head->next;
        tmp=head;
        i=1;
        while(tmp)
        {
            tmp->NO=i;
            tmp=tmp->next;
            ++i;
        }
    }
    else
    {
        tmp=head;
        i=1;
        while(tmp)
        {
            tmp->NO=i;
            ++i;
            if(tmp->next==wp_to_delete)
            {
                tmp->next=tmp->next->next;
            }
            tmp=tmp->next;
        }
    }
    tmp=free_;
    while(tmp->next)
	    tmp=tmp->next;
    tmp->next=wp_to_delete;
    
    wp_to_delete->NO=0;
    for(i=0;i<32;++i)
	    wp_to_delete->str[i]='\0';
    tmp->next->next=NULL;
    return;
}

WP* wp_head()
{
    return head;
}
