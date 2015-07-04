/*


*/

#include "sched/sched.h"


task_t g_tf[64];

volatile task_t *g_current_task = NULL;


void insert_new_task_into_run_queue(task_t * const t)
{
	ku16 x = SCHED_DISABLE();

	t->next = g_current_task->next;
	g_current_task->next = t;

	SCHED_ENABLE(x);
}


void remove_task_from_run_queue(const task_t *t)
{
	/* only remove a task if there is more than one task in the queue */
	if(t->next != t)
	{
		task_t *t_;
		
		/* locate the task before the task to be removed */
		for(t_ = t->next; t_->next != t; t_ = t_->next) ;
		
		/* update its next ptr to bypass the task to be removed */
		t_->next = t->next;
	}
}

