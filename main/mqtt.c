#include "mqtt.h"
#include <stdio.h>


#include <string.h>

#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "mqtt_client.h"


#define ALIIOT_MQTT_POST "/sys/gkfxtToavD9/leg_rehalibitation/thing/event/property/post"
#define ALIIOT_MQTT_POST_REPLY "/sys/gkfxtToavD9/leg_rehalibitation/thing/event/property/post_reply"
#define ALIIOT_MQTT_COMMUNICATION "/gkfxtToavD9/leg_rehalibitation/user/communication"

extern bool connect_sta;
bool mqtt_get_in = false;
static const char *TAG = "MQTT_EXAMPLE";

esp_mqtt_client_handle_t client;
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            mqtt_get_in = true;
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            ///gkfxtToavD9/${deviceName}/user/communication发布和订阅一起  ，自定义
            ///sys/gkfxtToavD9/leg_rehalibitation/thing/event/property/post发布  从机发送？
            msg_id = esp_mqtt_client_publish(client, ALIIOT_MQTT_POST, "{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"X_Roll_Angle\":0.0,\"Y_Pitch_Angle\":0.0,\"Z_Yaw_Angle\":0.0},\"version\":\"1.0\"}", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            msg_id = esp_mqtt_client_publish(client, ALIIOT_MQTT_COMMUNICATION, "i get in", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            ///sys/gkfxtToavD9/leg_rehalibitation/thing/event/property/post_reply订阅
            msg_id = esp_mqtt_client_subscribe(client, ALIIOT_MQTT_POST_REPLY, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
    	.host = "gkfxtToavD9.iot-as-mqtt.cn-shanghai.aliyuncs.com",
		.port = 1883,
		.client_id = "liujun|securemode=3,signmethod=hmacsha1|",
		.username = "leg_rehalibitation&gkfxtToavD9",
		.password = "3659E762D941B70F4F72609019619C2168A9A944",
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

     client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}
void send_to_aliyuniot(float get_x_angle)
{
    if(mqtt_get_in == true)
    {
        int msg_id;
        char *str = (char *)malloc (sizeof(char)*256);
        memset(str,0,256);
        //,(float)stcAngle.Angle[0]/32768*180,(float)stcAngle.Angle[1]/32768*180,(float)stcAngle.Angle[2]/32768*180
        //sprintf(str,"{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"X_Roll_Angle\":%.1f,\"Y_Pitch_Angle\":%.1f,\"Z_Yaw_Angle\":%.1f},\"version\":\"1.0\"}",(float)data.All_Angle.Angle[0]/32768*180,(float)data.All_Angle.Angle[1]/32768*180,(float)data.All_Angle.Angle[2]/32768*180);
        sprintf(str,"{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"X_Roll_Angle\":%.1f},\"version\":\"1.0\"}",get_x_angle);
        msg_id = esp_mqtt_client_publish(client, ALIIOT_MQTT_POST, str, 0, 1, 0);
        //esp_mqtt_client_publish(client, ALIIOT_MQTT_POST, "{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"X_Roll_Angle\":0.0,\"Y_Pitch_Angle\":0.0,\"Z_Yaw_Angle\":0.0},\"version\":\"1.0\"}", 0, 1, 0);
        printf("str:%s\r\n",str);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        printf("free_heap_size = %d\n", esp_get_free_heap_size());
        free(str);
        str=NULL;
    }
}
bool send_to_aliyun_results(leg_test_results result_data)//发给阿里云的最终数据
{
    if(mqtt_get_in == true)
    {
    int msg_id,i;
	char *str = (char *)malloc(sizeof(char)*512);
    memset(str,0,512);
    char str1[10]={0}, str2[20]={0}, str3[10]={0}, str4[20]={0};
	//,(float)stcAngle.Angle[0]/32768*180,(float)stcAngle.Angle[1]/32768*180,(float)stcAngle.Angle[2]/32768*180
#if 1 
	//sprintf(str,"{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"X_Roll_Angle\":%.1f,\"Y_Pitch_Angle\":%.1f,\"Z_Yaw_Angle\":%.1f},\"version\":\"1.0\"}",(float)data.All_Angle.Angle[0]/32768*180,(float)data.All_Angle.Angle[1]/32768*180,(float)data.All_Angle.Angle[2]/32768*180);
	sprintf(str, "{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"Prv_Per_Angle\":%d,\"Prv_Count\":%d,\"Prv_Leg_Freq\":%.1f,\"Prv_Test_Time\":%d,\"Time_Sum\":%d,\"Leg_Freq\":%.1f,\"Every_Leg_Time\":[%d", result_data.prv_per_angle, result_data.prv_count, result_data.prv_leg_freq, result_data.prv_time_sum, result_data.test_time_sum, result_data.leg_freq, result_data.lv_test_time[0]);
	//esp_mqtt_client_publish(client, ALIIOT_MQTT_POST, "{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"X_Roll_Angle\":0.0,\"Y_Pitch_Angle\":0.0,\"Z_Yaw_Angle\":0.0},\"version\":\"1.0\"}", 0, 1, 0);
	//printf("str:%s\n", str);
    for (i = 1; i < result_data.prv_count; i++)
	{
		sprintf(str1, ",%d", result_data.lv_test_time[i]);
		//printf("%d\n",strlen(str1));
		strncat(str,str1,strlen(str1));
		//printf("str:%s\n", str);
		//sprintf(&(str1[(i-1)]), ",%d", result_data.lv_test_time[i]);
	}
	sprintf(str2, "],\"Leg_Angle\":[%.1f", result_data.lv_test_angle[0]);
	strncat(str, str2, strlen(str2));
	printf("str:%s\n", str);
	for (i = 1; i < result_data.prv_count; i++)
	{
		sprintf(str3, ",%.1f", result_data.lv_test_angle[i]);
		strncat(str, str3, strlen(str3));
		//printf("str:%s\n", str);
	}
	printf("str3:%s\n", str);
	sprintf(str4, "]},\"version\":\"1.0\"}");
	strncat(str, str4, strlen(str4));
    printf("str:%s\r\n",str);
#endif     
 msg_id = esp_mqtt_client_publish(client, ALIIOT_MQTT_POST, str, 0, 1, 0);
        //esp_mqtt_client_publish(client, ALIIOT_MQTT_POST, "{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"X_Roll_Angle\":0.0,\"Y_Pitch_Angle\":0.0,\"Z_Yaw_Angle\":0.0},\"version\":\"1.0\"}", 0, 1, 0);
    printf("str:%s\r\n",str);
        free(str);
        str=NULL;
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        printf("free_heap_size = %d\n", esp_get_free_heap_size());

        if(msg_id>0)
        return true;
        else
        return false;
    }
    printf("aliyun get failed\n");
    return false;
}

bool send_to_aliyun_chart(leg_test_results result_data)
{
    if(mqtt_get_in == true)
    {
            int msg_id = 0,i;
        char *str = (char *)malloc (sizeof(char)*256);
        //,(float)stcAngle.Angle[0]/32768*180,(float)stcAngle.Angle[1]/32768*180,(float)stcAngle.Angle[2]/32768*180
        //sprintf(str,"{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"X_Roll_Angle\":%.1f,\"Y_Pitch_Angle\":%.1f,\"Z_Yaw_Angle\":%.1f},\"version\":\"1.0\"}",(float)data.All_Angle.Angle[0]/32768*180,(float)data.All_Angle.Angle[1]/32768*180,(float)data.All_Angle.Angle[2]/32768*180);
        for (i = 0; i < result_data.prv_count; i++)
        {
            memset(str,0,256);
            sprintf(str,"{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"Trans_Leg_Angle\":%.1f,\"Trans_Leg_Time\":%d},\"version\":\"1.0\"}",result_data.lv_test_angle[i],result_data.lv_test_time[i]);
            msg_id = esp_mqtt_client_publish(client, ALIIOT_MQTT_POST, str, 0, 1, 0);
            printf("str:%s\r\n",str);
            vTaskDelay(300);
        }
        //esp_mqtt_client_publish(client, ALIIOT_MQTT_POST, "{\"id\":\"230788029\",\"method\":\"thing.event.property.post\",\"params\":{\"X_Roll_Angle\":0.0,\"Y_Pitch_Angle\":0.0,\"Z_Yaw_Angle\":0.0},\"version\":\"1.0\"}", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        printf("free_heap_size = %d\n", esp_get_free_heap_size());
        free(str);
        str=NULL;

        if(msg_id>0)
        return true;
        else
        return false;
    }
        return false;
}