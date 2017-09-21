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
	WP* nwp = free_;
	free_ = free_->next;
	if (head == NULL)
		head = nwp;
	else 
	{
		WP* cwp = head;
		while (cwp->next != NULL)
			cwp = cwp->next;
		cwp->next = nwp;
	}
	nwp->next = NULL;
	return nwp;
}

void free_wp(WP *wp)
{
	WP* cwp = head;
	if (cwp == wp)
	{
		if (wp->next == NULL)
			cwp = NULL;
		else cwp = wp->next;
	}
	while (cwp != NULL)
	{
		if (cwp->next == wp)
		{
			if (wp->next == NULL)
				cwp->next = NULL;
			else cwp->next = wp->next;
			break;
		}
		else
			cwp = cwp->next;
	}
	if (cwp == NULL)
		Log("Watchpoint unfound\n");
	else
	{
		cwp = free_;
		while (cwp->next != NULL)
			cwp = cwp->next;
		cwp->next = wp;
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

/* TODO: Implement the functionality of watchpoint */


