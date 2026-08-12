#ifndef WORKQUEUE__H
#define WORKQUEUE__H
typedef void (*work_t)(void *param);
void workqueue_init(void);
void workqueue_run(work_t work,void*param);
#endif
