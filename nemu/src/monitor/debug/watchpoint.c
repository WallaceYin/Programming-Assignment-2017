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

WP* new_up()
{
	WP* nwp;
	WP* cwp;
	nwp = free_;
	free_ = free_->next;
	if (head == NULL)
		head = nwp;
	else 
	{
		cwp = head;
		while (cwp->next != NULL)
			cwp = cwp->next;
		cwp->next = nwp;
	}
	nwp->next = NULL;
	return nwp;
}

void free_wp(WP *wp)
{
	WP* cwp;
	cwp = head;
	bool fd = 0;
	if (cwp == wp)
	{
		Log("find watchpoint1 %d\n",wp->NO);
		if (wp->next == NULL)
			head = NULL;
		else 
		{
			head = wp->next;
			wp->next = NULL;
		}
		fd = 1;
	}//the watchpoint we wanna remove is on the head position
	else
		while (cwp->next != NULL)
		{
			if (cwp->next == wp)
			{
				Log("find watchpoint2 %d\n",wp->NO);
				if (wp->next == NULL)
					cwp->next = NULL;
				else
				{	
					cwp->next = wp->next;
					wp->next = NULL;
				}
				break;
				fd = 1;
			}
			else
				cwp = cwp->next;
		}
	//Remove the watchpoints in the detected list
	cwp = free_;
	while (cwp->next != NULL)
		cwp = cwp->next;
	if (fd)
	{
		cwp->next = wp;
		wp->next = NULL;
	}
	return;
}

WP* print_wp(WP *wp)
{
	if (wp == NULL)
		return head;
	else
		return wp->next;
}

bool check_wp()
{
	bool succ;
	int val;
	WP* wp;
	wp = head;
	bool rm = 1;
	while (wp != NULL)
	{
		val = expr(wp->exp,&succ);
		if (val != wp->Val)
		{
			printf("watchpoint %2d is stimulated, the value of %s has changed from %8u to %8u\n",wp->NO, wp->exp, wp->Val, val);
			wp->Val = val;
			rm = 0;
		}
		wp = wp->next;
	}
	if (!rm) return 0;
	else return 1;
}

/* TODO: Implement the functionality of watchpoint */


