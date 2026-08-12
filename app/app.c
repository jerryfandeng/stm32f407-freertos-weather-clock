#include "workqueue.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "rtc.h"
#include "aht20.h"
#include "esp_at.h"
#include "weather.h"
#include "page.h"
#include "app.h"
#include "elog.h"

#define MILLISECONDS(x) (x)
#define SECONDS(x)      MILLISECONDS((x) * 1000)
#define MINUTES(x)      SECONDS((x) * 60)
#define HOURS(x)        MINUTES((x) * 60)
#define DAYS(x)          HOURS((x) * 24)

#define TIME_SYNC_INTERVAL          HOURS(1)
#define WIFI_UPDATE_INTERVAL        SECONDS(5)
#define TIME_UPDATE_INTERVAL        SECONDS(1)
#define INNER_UPDATE_INTERVAL       SECONDS(3)
#define OUTDOOR_UPDATE_INTERVAL     MINUTES(1)

#define MLOOP_EVT_TIME_SYNC         (1 << 0)
#define MLOOP_EVT_WIFI_UPDATE       (1 << 1)
#define MLOOP_EVT_TIME_UPDATE       (1 << 2)
#define MLOOP_EVT_INNER_UPDATE      (1 << 3)
#define MLOOP_EVT_OUTDOOR_UPDATE    (1 << 4)
#define MLOOP_EVT_ALL               (MLOOP_EVT_TIME_SYNC | \
                                     MLOOP_EVT_WIFI_UPDATE | \
                                     MLOOP_EVT_TIME_UPDATE | \
                                     MLOOP_EVT_INNER_UPDATE | \
                                     MLOOP_EVT_OUTDOOR_UPDATE)


static TimerHandle_t time_sync_timer;
static TimerHandle_t wifi_update_timer;
static TimerHandle_t time_update_timer;
static TimerHandle_t inner_update_timer;
static TimerHandle_t outdoor_update_timer;

static void time_sync(void)
{
    uint32_t restart_sync_delay = TIME_SYNC_INTERVAL;
    rtc_date_time_t rtc_date = { 0 };

    esp_date_time_t esp_date = { 0 };
    if (!esp_at_sntp_get_time(&esp_date))
    {
        elog_e("SNTP", "get time failed");
        restart_sync_delay = SECONDS(1);
        goto err;
    }
    
    if (esp_date.year < 2000)
    {
        elog_w("SNTP", "invalid date format");
        restart_sync_delay = SECONDS(1);
        goto err;
    }
    
    elog_i("SNTP", "sync time: %04u-%02u-%02u %02u:%02u:%02u (%d)",
        esp_date.year, esp_date.month, esp_date.day,
        esp_date.hour, esp_date.minute, esp_date.second, esp_date.weekday);
    
    rtc_date.year = esp_date.year;
    rtc_date.month = esp_date.month;
    rtc_date.day = esp_date.day;
    rtc_date.hour = esp_date.hour;
    rtc_date.minute = esp_date.minute;
    rtc_date.second = esp_date.second;
    rtc_date.weekday = esp_date.weekday;
    rtc_set_time(&rtc_date);
    
err:
    xTimerChangePeriod(time_sync_timer, pdMS_TO_TICKS(restart_sync_delay), 0);
}

static void wifi_update(void)
{
    static esp_wifi_info_t last_info = { 0 };

    xTimerChangePeriod(wifi_update_timer, pdMS_TO_TICKS(WIFI_UPDATE_INTERVAL), 0);
    
    esp_wifi_info_t info = { 0 };
    if (!esp_at_get_wifi_info(&info))
    {
        elog_e("AT", "wifi info get failed");
        return;
    }
    
    if (memcmp(&info, &last_info, sizeof(esp_wifi_info_t)) == 0)
    {
        return;
    }
    
    if (last_info.connected == info.connected)
    {
        return;
    }
    
    if (info.connected)
    {
        elog_i("WIFI", "connected to %s", info.ssid);
        elog_i("WIFI", "SSID: %s, BSSID: %s, Channel: %d, RSSI: %d",
                info.ssid, info.bssid, info.channel, info.rssi);
        main_page_redraw_wifi_ssid(info.ssid);
    }
    else
    {
        elog_i("WIFI", "disconnected from %s", last_info.ssid);
        main_page_redraw_wifi_ssid("wifi lost");
    }
    
    memcpy(&last_info, &info, sizeof(esp_wifi_info_t));
}

static void time_update(void)
{
    static rtc_date_time_t last_date = { 0 };
    
    xTimerChangePeriod(time_update_timer, pdMS_TO_TICKS(TIME_UPDATE_INTERVAL), 0);
    
    rtc_date_time_t date;
    rtc_get_time(&date);
    
    if (date.year < 2020)
    {
        return;
    }
    
    if (memcmp(&date, &last_date, sizeof(rtc_date_time_t)) == 0)
    {
        return;
    }
    
    memcpy(&last_date, &date, sizeof(rtc_date_time_t));
    main_page_redraw_time(&date);
    main_page_redraw_date(&date);
}

static void inner_update(void)
{
    static float last_temperature, last_humidity;
    
    xTimerChangePeriod(inner_update_timer, pdMS_TO_TICKS(INNER_UPDATE_INTERVAL), 0);
    
    if (!aht20_start_measurement())
    {
        elog_e("AHT20", "start measurement failed");
        return;
    }
    
    if (!aht20_wait_for_measurement())
    {
        elog_e("AHT20", "wait for measurement failed");
        return;
    }
    
    float temperature = 0.0f, humidity = 0.0f;
    
    if (!aht20_read_measurement(&temperature, &humidity))
    {
        elog_e("AHT20", "read measurement failed");
        return;
    }
    
    if (temperature == last_temperature && humidity == last_humidity)
    {
        return;
    }
    
    last_temperature = temperature;
    last_humidity = humidity;
    
    elog_i("AHT20", "Temperature: %.1f, Humidity: %.1f", temperature, humidity);
    main_page_redraw_inner_temperature(temperature);
    main_page_redraw_inner_humidity(humidity);
}

static void outdoor_update(void)
{
    static weather_info_t last_weather = { 0 };
    
    xTimerChangePeriod(outdoor_update_timer, pdMS_TO_TICKS(OUTDOOR_UPDATE_INTERVAL), 0);
    
    weather_info_t weather = { 0 };
    const char *weather_url = "https://api.seniverse.com/v3/weather/now.json?key=SfRic8Wmp-Qh3OeFk&location=WTEMH46Z5N09&language=en&unit=c";
    const char *weather_http_response = esp_at_http_get(weather_url);
    if (weather_http_response == NULL)
    {
        elog_e("WEATHER", "http error");
        return;
    }
    
    if (!parse_seniverse_response(weather_http_response, &weather))
    {
        elog_e("WEATHER", "parse failed");
        return;
    }
    
    if (memcmp(&last_weather, &weather, sizeof(weather_info_t)) == 0)
    {
        return;
    }
    
    memcpy(&last_weather, &weather, sizeof(weather_info_t));
    elog_i("WEATHER", "%s, %s, %.1f", weather.city, weather.weather, weather.temperature);
    
    main_page_redraw_outdoor_temperature(weather.temperature);
    main_page_redraw_outdoor_weather_icon(weather.weather_code);
}
typedef void (*app_job_t)(void);

static void app_func(void *pararm)
{
    app_job_t job = (app_job_t)pararm;
    job();
}

static void app_timer_cb(TimerHandle_t timer)
{
    app_job_t job = (app_job_t)pvTimerGetTimerID(timer);
    job();
}

static void work_timer_cb(TimerHandle_t timer)
{
    app_job_t job = (app_job_t)pvTimerGetTimerID(timer);
    workqueue_run(app_func,job);
}

void  app_init(void)
{
    time_update_timer = xTimerCreate("time update", pdMS_TO_TICKS(TIME_UPDATE_INTERVAL), pdTRUE, time_update, app_timer_cb);
    time_sync_timer = xTimerCreate("time sync", 1, pdFALSE,time_sync, work_timer_cb);
    wifi_update_timer = xTimerCreate("wifi update", pdMS_TO_TICKS(WIFI_UPDATE_INTERVAL), pdTRUE, wifi_update, work_timer_cb);
    outdoor_update_timer = xTimerCreate("outdoor update", pdMS_TO_TICKS(OUTDOOR_UPDATE_INTERVAL), pdTRUE, outdoor_update, work_timer_cb);
    inner_update_timer = xTimerCreate("inner upadte", pdMS_TO_TICKS(INNER_UPDATE_INTERVAL), pdTRUE, inner_update, work_timer_cb);
    
    //xTaskCreate(mloop_func, "mloop", 1024, NULL, 5, &mloop_task);
    //队列中执行app_func(time_sync)
    workqueue_run(app_func,time_sync);
    workqueue_run(app_func,wifi_update);
    //workqueue_run(app_func,time_update);
    workqueue_run(app_func,inner_update);
    workqueue_run(app_func,outdoor_update);

    
    xTimerStart(wifi_update_timer, 0);
    xTimerStart(time_update_timer, 0);
    xTimerStart(time_sync_timer, 0);
    xTimerStart(inner_update_timer, 0);
    xTimerStart(outdoor_update_timer, 0);
}
