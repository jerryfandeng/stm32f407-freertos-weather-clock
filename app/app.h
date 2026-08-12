#ifndef __APP_H__
#define __APP_H__

#define APP_VERSION "v1.0"
//使用你自己的热点名称和密码
#define WIFI_SSID   "YOUR_WIFI_SSID"
#define WIFI_PASSWD "YOUR_WIFI_PASSWORD"

void wifi_init(void);
void wifi_wait_connect(void);

void app_init(void);



#endif /* __APP_H__ */
