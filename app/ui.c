#include "st7789.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "stdio.h"
#include "string.h"
#include "ui.h"
//freertos队列传递信息
typedef enum
{
    UI_FILL_COLOR,
    UI_WRITE_STR,
    UI_DRAW_IMAGE,
}ui_action;

typedef struct
{
    ui_action action;
    union 
    {
        struct 
        {
           uint16_t x;
           uint16_t y;
           uint16_t width;
           uint16_t height;
           uint16_t color;
        }fill_color;
        struct 
        {
            uint16_t x;
            uint16_t y;
            const char *str;
            uint16_t color;
            uint16_t bg_color;
            const font_t *font;
        }write_string;
        struct 
        {
            uint16_t x;
            uint16_t y; 
            const image_t *image;
        }draw_image;   
    };  
}ui_msg;

static QueueHandle_t ui_queue;

void ui_func(void *param)
{
    //接收信息
    ui_msg msg;
    st7789_init();
    while(1)
    {
        xQueueReceive(ui_queue,&msg,portMAX_DELAY);
        switch (msg.action)
        {
            case UI_FILL_COLOR:
                st7789_fill_color(msg.fill_color.x,msg.fill_color.y,msg.fill_color.width,msg.fill_color.height,msg.fill_color.color);
                break;
            case UI_DRAW_IMAGE:
                st7789_draw_image(msg.draw_image.x,msg.draw_image.y,msg.draw_image.image);
                break;
            case UI_WRITE_STR:
                st7789_write_string(msg.write_string.x,msg.write_string.y,msg.write_string.str,msg.write_string.color,msg.write_string.bg_color,msg.write_string.font);
                vPortFree((void*)msg.write_string.str);
                break;
            default:
                break;
            }
    }

}

void ui_init(void)
{
    ui_queue = xQueueCreate(16,sizeof(ui_msg));
    configASSERT(ui_queue);
    xTaskCreate(ui_func,"ui func",2048,NULL,5,NULL);
}
//将参数信息放入队列
void ui_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    ui_msg msg;
    msg.action = UI_FILL_COLOR;

    msg.fill_color.color = color;
    msg.fill_color.height = height;
    msg.fill_color.width = width;
    msg.fill_color.x = x;
    msg.fill_color.y = y;
    
    xQueueSend(ui_queue,&msg,portMAX_DELAY);
}

void ui_write_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color, const font_t *font)
{
    char*pstr;
    pstr = pvPortMalloc(strlen(str) + 1);
    if(pstr == NULL)
    {
        configASSERT(pstr);
    }
    strcpy(pstr,str);

    ui_msg msg;
    msg.action = UI_WRITE_STR;
    
    msg.write_string.bg_color = bg_color;
    msg.write_string.color = color;
    msg.write_string.font = font;
    msg.write_string.str = pstr;
    msg.write_string.x = x;
    msg.write_string.y = y;
    xQueueSend(ui_queue,&msg,portMAX_DELAY);
}

void ui_draw_image(uint16_t x, uint16_t y, const image_t *image)
{
    ui_msg msg;
    msg.action = UI_DRAW_IMAGE;
    
    msg.draw_image.image = image;
    msg.draw_image.x = x;
    msg.draw_image.y = y;

    xQueueSend(ui_queue,&msg,portMAX_DELAY);
}
