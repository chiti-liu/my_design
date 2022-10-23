/*
mode by xiaoxiaoyudu
github :https://github.com/xiaoxiaoyudu
qq :1907383069
blbl:两位真实好友
*/
#include "cJSON.h"
#include "updata.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"
#include "wifi_station.h"
#include <string.h>
#include "webserver.h"
#include <stdlib.h>
#include <malloc.h>

static const char *TAG="NVS";
extern char *user_id;
extern char *user_code;
extern bool connect_sta;
int save_nvs(const char * str,char * data)
{
    nvs_handle my_handle;//nvs句柄
    esp_err_t err = nvs_open("info", NVS_READWRITE, &my_handle);//打开wifipass空间
    err = nvs_set_str(my_handle,str,data);
    err =nvs_commit(my_handle);
    if(err==ESP_OK) 
        ESP_LOGI(TAG,"保存%s:%s",str,data);
    else
    {
        ESP_LOGI(TAG,"保存失败");  
        return 0;
    }
    nvs_close(my_handle);
    return 1;
}
int read_nvs(const char * str,char * data)
{
    /*打开wifipass工作区*/
    size_t len =40;
    nvs_handle my_handle;//nvs句柄
    esp_err_t err = nvs_open("info", NVS_READWRITE, &my_handle);//打开wifipass空间
    /*从nvs中获取到ssid和密码*/
    err =nvs_get_str(my_handle,str,data,&len);
    if(err==ESP_OK)
        ESP_LOGI(TAG,"读取%s:%s",str,data);
    else
    {
        ESP_LOGI(TAG,"读取失败"); 
        return 0;
    }  
    nvs_close(my_handle);
    return 1; 
}
/*
** 取出web端的WIFI账号和密码，校验和过滤，用到Cjson，这里特别要注意，
** cJSON_Delete会从根节点开始往下删，所以不要对子节点先删除，不然会造成内存泄漏，
** pvPortMalloc是从ZI data里面分配空间   malloc是从DRAM中分配空间   
** malloc和free要看是不是还对同一片地址操作，中间对指针变量有没有修改
*/
void save_web_data(char *data)
{
    char data_get[100]= {0},*data_temp= (char*)pvPortMalloc(sizeof(char)*1024);
    assert(data_temp != NULL);
    char *temp = data_temp;
    assert(temp != NULL);
    strncpy(data_temp,data,strlen(data));
    cJSON *root,*which;
    printf("free_heap_size = %d\n", esp_get_free_heap_size());
    //printf("updata  %s\r\n",data_temp);
    //data_get = strchr(data_get,'{');
    if(!data_temp)
    {
        printf("no!!!!!!!!!!!!!!!!!!\r\n");
        return ;
    }
    while(*data_temp!='{')
    {
        data_temp++;
        if((*data_temp)=='\0')
        {
            printf("no{!!!!!!!!!!!\r\n");
            if(temp!=NULL)
            {
                printf("delete\r\n");
                //data_temp = *temp ;
                vPortFree(temp);
                //vPortFree(data_temp); 
                data_temp =NULL;
                temp =NULL;
            }
            return ;
        }
    }
    int i =0 ;
    while(*data_temp!='}')
    {
        data_get[i++]=*data_temp;
        data_temp++;
        if((*data_get)=='\0'||i>99)
        {
            if(temp!=NULL)
            {
                printf("delete\r\n");
                //data_temp = *temp ;
                vPortFree(temp);
                //vPortFree(data_temp); 
                data_temp =NULL;
                temp =NULL;
            }
            printf("no_}!!!!!!!!!!!\r\n");
            return ;
        }
    }
    data_get[i]='}';
    root = cJSON_Parse(data_get);
    printf("updata  %s\r\n",data_get);
    if(root!=NULL)
    {
        printf("getin\r\n");
        which = cJSON_GetObjectItem(root, "n");
        if(which!=NULL)
        {
            if(which->valueint==1)//必填项
            {
                user_id = (char*)pvPortMalloc(sizeof(char)*32);
                user_code = (char*)pvPortMalloc(sizeof(char)*64);
                cJSON *name=cJSON_GetObjectItem(root,"wifi_name");//获取wifi名字
                int len = strlen(name->valuestring);
                printf("len:%d\r\n",len);
                memcpy(user_id,name->valuestring,strlen(name->valuestring));
                *(user_id+len)='\0';
                printf("wifi_name %s\r\n",user_id);
                //save_nvs("wifi_name",name->valuestring);
                cJSON *code=cJSON_GetObjectItem(root,"wifi_code");//获取密码
               // save_nvs("wifi_code",code->valuestring);
               len = strlen(code->valuestring);
                memcpy(user_code,code->valuestring,len);           
                printf("len:%d\r\n",len);
               *(user_code+len)='\0';
                printf("wifi_code %s\r\n",user_code);
                connect_sta = true;
            }
        }
    cJSON_Delete(root);   
    printf("getinto000ooo\r\n");    
    }
            if(temp!=NULL)
            {
                printf("delete\r\n");
                //data_temp = *temp ;
                vPortFree(temp);
                //vPortFree(data_temp); 
                data_temp =NULL;
                temp =NULL;
            }
    printf("getinto000o\r\n");
    return ;
}