/* license header */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "rtc.h"
#include <elog.h>
#include "console.h"
#include "stm32f4xx.h"
#include <string.h>
#include <stdio.h>

static SemaphoreHandle_t elog_mutex;

ElogErrCode elog_port_init(void)
{
    ElogErrCode result = ELOG_NO_ERR;
    elog_mutex = xSemaphoreCreateMutex();
    return result;
}

void elog_port_deinit(void)
{
    vSemaphoreDelete(elog_mutex);
}

void elog_port_output(const char *log, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, (uint8_t)log[i]);
    }
}

void elog_port_output_lock(void)
{
    if (elog_mutex != NULL) {
        xSemaphoreTake(elog_mutex, portMAX_DELAY);
    }
}

void elog_port_output_unlock(void)
{
    if (elog_mutex != NULL) {
        xSemaphoreGive(elog_mutex);
    }
}

const char *elog_port_get_time(void)
{
    static char time_str[32];
    rtc_date_time_t time;
    rtc_get_time(&time);
    if (time.year >= 2000 && time.year <= 2099) {
        snprintf(time_str, sizeof(time_str), "%02d-%02d-%02d %02d:%02d:%02d",
                 time.year % 100, time.month, time.day,
                 time.hour, time.minute, time.second);
        return time_str;
    } else {
        snprintf(time_str, sizeof(time_str), "%lu", xTaskGetTickCount());
        return time_str;
    }
}

const char *elog_port_get_p_info(void) { return ""; }

const char *elog_port_get_t_info(void)
{
    if (xTaskGetCurrentTaskHandle() != NULL)
        return pcTaskGetName(NULL);
    return "none";
}
