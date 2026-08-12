#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "console.h"
#include "rtc.h"
#include "aht20.h"
#include "st7789.h"
#include "FreeRTOS.h"
#include "task.h"
#include "elog.h"
#include "cm_backtrace.h"
#include "cmb_def.h"
#include "cmb_cfg.h"
void board_lowlevel_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    RCC_LSEConfig(RCC_LSE_ON);
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
}

void board_init(void)
{
    console_init();
    rtc_init();
    printf("[SYS] Build Date: %s %s\n", __DATE__, __TIME__);

    aht20_init();
}

int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    return ch;
}

void debug_putchar_polling(char c) {
    while((USART1->SR & USART_SR_TXE) == 0); 
    USART1->DR = c;
}

void vAssertCalled(const char *file, int ulLine)
{
    taskDISABLE_INTERRUPTS();
		cm_backtrace_assert(cmb_get_sp());
    log_a("ASSERT failed: %s, line %d\n", file, ulLine);
		while(1){}
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    while (1) { }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    portDISABLE_INTERRUPTS();
//		taskDISABLE_INTERRUPTS();
    cm_backtrace_assert(cmb_get_sp());
    while (1);
}



