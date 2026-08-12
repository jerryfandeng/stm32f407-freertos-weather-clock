#include "page.h"
#include "app.h"
#include "FreeRTOS.h"
#include "task.h" 
#include <stdio.h>
#include "tim_delay.h"
#include "ui.h"
#include "elog.h"
#include "workqueue.h"
#include "st7789.h"
#include "cm_backtrace.h"
#include "cmb_def.h"
#include "cmb_cfg.h"
extern void board_lowlevel_init(void);
extern void board_init(void);
//打印任务信息(包括任务名 任务状态 任务优先级 任务创建顺序)
static void state_probe(void*param)
{
	char buf[512];
	
	(void)param;
	for(;;)
	{
		vTaskList(buf);
		elog_i("probe","\r\n%s",buf);
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void main_init(void *param)
{
    tim_delay_init();
    board_init();

    elog_init();
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_T_INFO);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_start();
	
    ui_init();
    
    wifi_init();
    wifi_page_display();
    wifi_wait_connect();
     
    main_page_display();

    app_init();
    vTaskDelete(NULL);
}

int main(void)
{

   cm_backtrace_init("stm32f407", "V1.0.0", "V1.0.0");
   board_lowlevel_init();
   tim_delay_init();
   board_init();

   workqueue_init();
   xTaskCreate(main_init,"main init",512,NULL,5,NULL);
   xTaskCreate(state_probe,"state probe",512,NULL,0,NULL);
   vTaskStartScheduler();
}



