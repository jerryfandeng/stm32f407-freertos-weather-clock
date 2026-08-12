#include <stdint.h>
#include <stdio.h>
#include "tim_delay.h"
#include "esp_at.h"
#include "page.h"
#include "app.h"
#include "elog.h"
void wifi_init(void)
{
    if (!esp_at_init())
    {
        elog_e("AT", "init failed");
        goto err;
    }
    elog_i("AT", "inited");
    
    if (!esp_at_wifi_init())
    {
        elog_e("WIFI", "init failed");
        goto err;
    }
    elog_i("WIFI", "inited");
    
    if (!esp_at_sntp_init())
    {
        elog_e("SNTP", "init failed");
        goto err;
    }
    elog_i("SNTP", "inited");
    
    return;
    
err:
    error_page_display("wireless init failed");
    while (1)
    {
        ;
    }
}

void wifi_wait_connect(void)
{
    elog_i("WIFI", "connecting");
    
    esp_at_connect_wifi(WIFI_SSID, WIFI_PASSWD, NULL);
    
    for (uint32_t t = 0; t < 10 * 1000; t += 100)
    {
        tim_delay_ms(100);

        esp_wifi_info_t wifi = { 0 };
        if (esp_at_get_wifi_info(&wifi) && wifi.connected)
        {
            elog_i("WIFI", "Connected");
            elog_i("WIFI", "SSID: %s, BSSID: %s, Channel: %d, RSSI: %d",
                wifi.ssid, wifi.bssid, wifi.channel, wifi.rssi);
            return;
        }
    }
    
    elog_e("WIFI", "Connection Timeout");
    error_page_display("wireless connect failed");
    while (1)
    {
        ;
    }
}
