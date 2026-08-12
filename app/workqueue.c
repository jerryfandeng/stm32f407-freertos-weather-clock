#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "workqueue.h"

static QueueHandle_t workqueue;
typedef struct 
{
    work_t work;
    void*  param;
}work_message_t;

//msg.work(param)

void workqueue_run(work_t work,void*param)
{
    configASSERT(workqueue);
    work_message_t msg = {work,param};
    xQueueSend(workqueue,&msg,portMAX_DELAY);
}

void workqueue_func(void*param)
{
    work_message_t msg;
    while (1)
    {
        xQueueReceive(workqueue,&msg,portMAX_DELAY);
        msg.work(msg.param);
    }    
}

void workqueue_init(void)
{
    workqueue = xQueueCreate(16,sizeof(work_message_t));
    configASSERT(workqueue);
    xTaskCreate(workqueue_func,"workqueue",1024,NULL,5,NULL);
}

