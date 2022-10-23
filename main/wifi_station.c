#include <string.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <stdlib.h>
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "wifi_station.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"

#include "esp_http_client.h"
#include "mqtt.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "GAUSSIAN"
#define EXAMPLE_ESP_WIFI_PASS      "gaussian705"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

/* FreeRTOS event group to signal when we are connected*/
extern  EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static const char *TAG = "wifi station";
static wifi_config_t wifi_config;
static int s_retry_num = 0;
extern bool connect_sta;
extern void wifi_init_ap(void);
static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    wifi_mode_t mode;
    switch (event_id) {
    case IP_EVENT_STA_GOT_IP: {
        esp_wifi_get_mode(&mode);
         ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "wifi_connecttttttttttttttttttttttttttttttttttt\r\n");
        mqtt_app_start();//得到wifi开始MQTT
        
    }
    default:
        break;
    }
    return;
}

static void sta_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "ssid:%s    passward :%s\n",wifi_config.sta.ssid,wifi_config.sta.password);
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {

        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "ssid:%s    passward :%s,passssssssssssssssssssssss\n",wifi_config.sta.ssid,wifi_config.sta.password);
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            vTaskDelay(100);
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            connect_sta = false;
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else  {
        ESP_LOGI(TAG,"");
    }
    ESP_LOGI(TAG, "event base:  %s::::::::::::%d\n",event_base,event_id);
}

void wifi_init_sta(void)
{

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    /*初始化TCP/IP协议栈，已经初始化好*/
    //ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    
    /*创建默认事件循环，已经初始化好*/
    /*创建一个默认的WIFI-STA网络接口，如果初始化错误，此API将中止*/
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &sta_event_handler,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ip_event_handler,
                                                        NULL));
#if 0
     wifi_config = {
        .sta = {
            //.ssid = name,
            //.password = code,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
#endif 
#if 1
        int len = strlen(user_id);
        strncpy((char *)wifi_config.sta.ssid, user_id, len);
        //wifi_config.sta.ssid[len] = '\0';
        len = strlen(user_code);
        strncpy((char *)wifi_config.sta.password, user_code,len);
        //wifi_config.sta.password[len] = '\0';
#endif 
    //memcpy(wifi_config.sta.ssid,name,32);
    //memcpy(wifi_config.sta.password,code,64);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start());

     ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 user_id, user_code);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 user_id, user_code);
                 connect_sta = false;

    } else {
        ESP_LOGE(TAG, "UNEXPECTED Eprotocol_examples_common.h:ENT");
    }
#if 0
    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(wifi_event_group);
#endif
}