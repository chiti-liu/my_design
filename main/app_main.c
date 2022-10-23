#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "nvs_flash.h"

#include <netdb.h>
#include <sys/socket.h>

//#include "/compoents/dns_server/my_dns_server.h"
//#include "/compoents/webserver/webserver.h"
#include "my_dns_server.h"
#include "webserver.h"
#include "freertos/mpu_wrappers.h"
#include "driver/uart.h"
#include "driver/gpio.h"
//#include "st7735s.h"
#include "mpu6050.h"
#include "updata.h"
#include "lv_port_indev.h"
#include "lvgl_key.h"
//#include "lv_examples/lv_examples/src/lv_demo_leg_recover/lv_demo_leg_recover.h"
/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
#include "lv_examples/lv_examples/lv_demo.h"
#include "lvgl_helpers.h"
#include "wifi_station.h"
#include "dy_sv17f.h"
#define LV_TICK_PERIOD_MS 1
bool connect_sta;
/**********************
 *  STATIC PROTOTYPES
 **********************/
/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */

static const char *TAG = "Main";
static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void ctrl_AP_STA_task(void *pvParameter);


static esp_err_t ap_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    switch(event_id) 
    {
        case WIFI_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG,"a phone connected!");
            break;
        case WIFI_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG,"A Phone disconnect  ...");
        default:
            break;
    }
    return ESP_OK;
}

void wifi_init_ap(void)
{
        esp_netif_create_default_wifi_ap();
        /*初始化TCP/IP协议栈*/
    //ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &ap_event_handler,
                                                        NULL));
    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = "自动弹出页面",
            .ssid_len = strlen("自动弹出页面"),
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = 3,
        },
    };

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP));

    ESP_LOGI(TAG, "Setting WiFi softAP SSID %s...", wifi_ap_config.ap.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}


void app_main(void)
{
    connect_sta = false;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    button_init();
    dy_sv17f_uart_init();
    printf("free_heap_size = %d\n", esp_get_free_heap_size());
    wifi_init_ap();   //初始化wifi
    xTaskCreatePinnedToCore(ctrl_AP_STA_task, "ctrl_AP_STA_task", 2048*2, NULL, 8, NULL, 1);//决定开启AP还是STA
    //wifi_init_sta();
    //st7735s_init();
    mpu6050_uart_init();
    dns_server_start();  //开启DNS服务
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*6, NULL, 2, NULL, 0);
    web_server_start();  //开启http服务
    printf("free_heap_size = %d\n", esp_get_free_heap_size());
}

static void ctrl_AP_STA_task(void *pvParameter)
{
    static bool prv = false;
    while(1)
    {
        /*
        ** 这里依旧有问题，切换不回来AP模式，可以考虑模式共存
        ** 共存有问题  切换不成  后面执行不到这里
        **共存切换可能 资源访问互斥用二值信号量好点   
        */
       vTaskDelay(pdMS_TO_TICKS(30));
        if(prv!=connect_sta)
        {
            if(connect_sta == true)
            wifi_init_sta();
            else
            wifi_init_ap();   //初始化wifi
            prv=connect_sta;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
        vTaskDelete(NULL);
}
/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;
lv_indev_t * my_indev;
static void guiTask(void *pvParameter) {

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    /* Use double buffered when not working with monochrome displays */
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);
    //lv_color_t* buf2 = NULL;
    static lv_disp_draw_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

#if defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_IL3820         \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_JD79653A    \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_UC8151D     \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_SSD1306

    /* Actual size in pixels, not bytes. */
    size_in_px *= 8;
#endif

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res=LV_HOR_RES_MAX;
    disp_drv.ver_res=LV_VER_RES_MAX;
    disp_drv.flush_cb = disp_driver_flush;

    /* When using a monochrome display we need to register the callbacks:
     * - rounder_cb
     * - set_px_cb */

    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);


    lv_port_indev_init();

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Create the demo application */
    lv_demo_leg_recover();

    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));
        //page_switch();
        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            //printf("free_heap_size = %d\n", esp_get_free_heap_size());
            xSemaphoreGive(xGuiSemaphore);
       }
    }
    /* A task should NEVER return */
    free(buf1);
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    free(buf2);
#endif
    vTaskDelete(NULL);
}

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}
