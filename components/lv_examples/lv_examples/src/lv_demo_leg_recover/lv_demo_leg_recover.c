/**
 * @file lv_demo_keypad_encoder.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../lv_demo.h"
#include "driver/gpio.h"
#include "lv_demo_leg_recover.h"
#include "lv_port_indev.h"
#include <stdio.h>
#include <string.h>
#include "dy_sv17f.h"
#include "mpu6050.h"
#include "lv_font_source_han_sans_bold_28.c"
#if LV_USE_DEMO_LEG_RECOVER

/*********************
 *      DEFINES
 *********************/
#define HIDE_WIN (0)
#define KEY_BACK 4
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
}disp_size_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_group_t *g;

static lv_obj_t *spinbox1;

static lv_obj_t * title_list;
//训练过程中变量
static lv_obj_t * bar_step;
static lv_obj_t *drop_count_dec;
static lv_obj_t *drop_count_dig;
static lv_obj_t *drop_angle_dec;
static lv_obj_t *drop_angle_dig;
static lv_obj_t *drop_time_dec;
static lv_obj_t *drop_time_dig;
//end 训练过程中变量

static lv_obj_t * tit_img1;
static lv_obj_t * tit_img2;
static lv_obj_t * tit_img3;

static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_icon;

static lv_obj_t* main_desktop;
static lv_obj_t* desktop_settings;
static lv_obj_t* desktop_records;
static lv_obj_t* desktop_voice;

static void btn1_toggle_event_cb(lv_event_t * e);
static void btn2_toggle_event_cb(lv_event_t * e);
static void btn3_toggle_event_cb(lv_event_t * e);
static void btn_start_toggle_event_cb(lv_event_t * e);
static void button_Task(void *pvParameter) ;

static void main_page(void);
static void tit_create(void);

static void page_settings(void);
static void page_records(void);
static void page_voice(void);
static uint8_t flag = 0;
static  lv_coord_t tab_h = 0;

static void obj_bar_anim_exec_callback(void * bar, int32_t value);
/**********************
 *      EXTERN VARIABLE
 **********************/
volatile bool test_start_flag = false;
volatile bool test_end_flag = false;

uint8_t count_sum = 0;//有个默认参数  保证为0的时候显示正常
uint8_t angle_per = 0;
uint32_t time_sum = 0;//预设参数分钟


LV_FONT_DECLARE(lv_font_source_han_sans_bold_28);                         // 声明外部字库
//extern lv_font_t lv_font_source_han_sans_bold_28;
 /**********************
 *   GLOBAL FUNCTIONS
 **********************/

 void lv_demo_leg_recover(void)
 {   
    g = lv_group_create();
    lv_group_set_default(g);

    // 将输入设备和组关联
     lv_indev_set_group(indev_keypad, g);     // 键盘

    xTaskCreatePinnedToCore(button_Task, "button_Task", 2046, NULL, 7, NULL,0);

    //tit_create();有问题
     main_page();

#if HIDE_WIN
     page_settings();
     lv_obj_add_flag(desktop_settings, LV_OBJ_FLAG_HIDDEN);
     page_records();
     lv_obj_add_flag(desktop_records, LV_OBJ_FLAG_HIDDEN);
     page_voice();
     lv_obj_add_flag(desktop_voice, LV_OBJ_FLAG_HIDDEN);
#endif

 }

  static void tit_create(void)
 {
    title_list = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);  //如果内存小可以考虑删掉头部状态栏

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_12);

    lv_style_init(&style_icon);
    lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
    lv_style_set_text_font(&style_icon, &lv_font_montserrat_12);

    lv_obj_t * tab_btns = lv_tabview_get_tab_btns(title_list);
    lv_obj_set_style_pad_left(tab_btns, LV_HOR_RES/2, 0);
    lv_obj_t * logo = lv_img_create(tab_btns);
    LV_IMG_DECLARE(img_lvgl_logo);
    lv_img_set_src(logo, &img_lvgl_logo);
    lv_obj_set_size(tab_btns,LV_HOR_RES,tab_h);
    //lv_obj_set_size(logo,40,18);
    //lv_obj_add_flag(title_list,LV_OBJ_FLAG_PRESS_LOCK);
    //lv_obj_clear_flag(tab_btns,LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_align(logo, LV_ALIGN_LEFT_MID,-LV_HOR_RES / 2 + 10, 0);

    lv_obj_t * label = lv_label_create(tab_btns);
    lv_label_set_text(label, "Leg_Rehabilitation");
    lv_obj_add_style(label, &style_text_muted, 0);
    lv_obj_align_to(label, logo, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);

    label = lv_label_create(tab_btns);
    lv_label_set_text(label, "made by L_J");
    lv_obj_add_style(label, &style_text_muted, 0);
    lv_obj_align_to(label, logo, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 0);

    //lv_obj_t * tab_btns2 = lv_tabview_get_tab_btns(title_list);  //右部状态栏
    //lv_obj_set_style_pad_left(tab_btns2, LV_HOR_RES / 2, 0);

    tit_img1 = lv_img_create(tab_btns);
    lv_img_set_src(tit_img1, LV_SYMBOL_BATTERY_FULL);
    lv_obj_align(tit_img1, LV_ALIGN_RIGHT_MID, 0, 0);

    tit_img2 = lv_img_create(tab_btns);
    lv_img_set_src(tit_img2, LV_SYMBOL_WIFI);
    lv_obj_align(tit_img2, LV_ALIGN_RIGHT_MID, -20, 0);

 }
static void  btn1_toggle_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);        // ��ȡ�����¼��Ĳ���(����)
    lv_event_code_t code = lv_event_get_code(e);    // ��ȡ��ǰ����(����)�������¼�����

    switch(code){
        case LV_EVENT_PRESSED:
#if HIDE_WIN
            // ������1�����������������ԣ��������2����������
            lv_obj_add_flag(main_desktop, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(desktop_settings, LV_OBJ_FLAG_HIDDEN);
            printf("LV_EVENT_PRESSED_11111\n");
#else
            lv_obj_del(main_desktop);
            page_settings();
#endif
            flag = 1;
            printf("LV_EVENT_PRESSED_1\n");
            break;
        default:
            //printf("NONE\n");
            break;
    }

}

 static void btn2_toggle_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);        // ��ȡ�����¼��Ĳ���(����)
    lv_event_code_t code = lv_event_get_code(e);    // ��ȡ��ǰ����(����)�������¼�����

    switch(code){
        case LV_EVENT_PRESSED:
#if HIDE_WIN
            lv_obj_add_flag(main_desktop, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(desktop_records, LV_OBJ_FLAG_HIDDEN);
#else
            lv_obj_del(main_desktop);
            page_records();
#endif
            flag = 2;
            printf("LV_EVENT_PRESSED_2\n");
            break;
        default:
            //printf("NONE\n");
            break;
    }
}

 static void btn3_toggle_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);        // ��ȡ�����¼��Ĳ���(����)
    lv_event_code_t code = lv_event_get_code(e);    // ��ȡ��ǰ����(����)�������¼�����

    switch(code){
        case LV_EVENT_PRESSED:
# if HIDE_WIN
            lv_obj_add_flag(main_desktop, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(desktop_voice, LV_OBJ_FLAG_HIDDEN);
#else
            lv_obj_del(main_desktop);
            page_voice();
#endif
            flag = 3;
            printf("LV_EVENT_PRESSED_3\n");
            break;
        default:
            //printf("NONE\n");
            break;
    }
}

static void main_page(void)
{
    main_desktop = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_desktop, LV_HOR_RES, LV_VER_RES-tab_h);
    lv_obj_align(main_desktop, LV_ALIGN_BOTTOM_MID, 0,0);

    /* ����һ��btn����(����) */
    lv_obj_t * btn1 = lv_btn_create(main_desktop);       // ����һ��btn1����(����),���ĸ������ǻ��Ļ����
    lv_obj_set_size(btn1, LV_HOR_RES/2, LV_HOR_RES/4);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t * label1 = lv_label_create(btn1);
    //lv_label_set_text(label1, "settings");
    lv_obj_set_style_text_font(label1, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label1, "设置");
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * img1 = lv_img_create(main_desktop);
    lv_img_set_src(img1, LV_SYMBOL_SETTINGS);
    lv_obj_align_to(img1, btn1, LV_ALIGN_OUT_LEFT_MID, -20, 0);

    // ���� LV_EVENT_PRESSED �¼�����ʾ��
    lv_obj_add_event_cb(btn1, btn1_toggle_event_cb, LV_EVENT_PRESSED, NULL);


    lv_obj_t * btn2 = lv_btn_create(main_desktop);       // ����һ��btn2����(����),���ĸ������ǻ��Ļ����
    lv_obj_set_size(btn2, LV_HOR_RES/2,  LV_HOR_RES/4);
    lv_obj_align_to(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    lv_obj_t * label2 = lv_label_create(btn2);
    //lv_label_set_text(label2, "records");
    lv_obj_set_style_text_font(label2, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label2, "记录");
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * img2 = lv_img_create(main_desktop);
    lv_img_set_src(img2, LV_SYMBOL_EYE_OPEN);
    lv_obj_align_to(img2, btn2, LV_ALIGN_OUT_LEFT_MID, -20, 0);


    // ���� LV_EVENT_PRESSED �¼�����ʾ��
    lv_obj_add_event_cb(btn2, btn2_toggle_event_cb, LV_EVENT_PRESSED, NULL);

    lv_obj_t * btn3 = lv_btn_create(main_desktop);       // ����һ��btn3����(����),���ĸ������ǻ��Ļ����
    lv_obj_set_size(btn3,  LV_HOR_RES/2,  LV_HOR_RES/4);
    lv_obj_align_to(btn3, btn2, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    lv_obj_t * label3 = lv_label_create(btn3);
    //lv_label_set_text(label3, "voice");
    lv_obj_set_style_text_font(label3, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label3, "声音");
    lv_obj_align_to(label3, btn3, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * img3 = lv_img_create(main_desktop);
    lv_img_set_src(img3, LV_SYMBOL_BELL);
    lv_obj_align_to(img3, btn3, LV_ALIGN_OUT_LEFT_MID, -20, 0);

    // ���� LV_EVENT_PRESSED �¼�����ʾ��
    lv_obj_add_event_cb(btn3, btn3_toggle_event_cb, LV_EVENT_PRESSED, NULL);
}

static void page_settings(void)
{
    desktop_settings = lv_obj_create(lv_scr_act());
    lv_obj_set_size(desktop_settings, LV_HOR_RES, LV_VER_RES-tab_h);
    lv_obj_align(desktop_settings, LV_ALIGN_BOTTOM_MID, 0,0);
# if 1
    drop_count_dec = lv_dropdown_create(desktop_settings);
    lv_obj_add_flag(drop_count_dec, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_align(drop_count_dec, LV_ALIGN_CENTER, 10,-30);
    lv_obj_set_size(drop_count_dec,LV_HOR_RES/5,LV_HOR_RES/5);
    lv_dropdown_set_symbol(drop_count_dec,NULL);
    lv_dropdown_set_options(drop_count_dec, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");//十位

    drop_count_dig = lv_dropdown_create(desktop_settings);
    lv_obj_add_flag(drop_count_dig, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(drop_count_dig,LV_HOR_RES/5,LV_HOR_RES/5);
    lv_dropdown_set_symbol(drop_count_dig,NULL);
    lv_obj_align_to(drop_count_dig,drop_count_dec, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_dropdown_set_options(drop_count_dig, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");//个位
    //lv_dropdown_get_option_cnt();

    lv_obj_t * label1 = lv_label_create(desktop_settings);
    lv_obj_set_style_text_font(label1, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label1, "次数:");
    //lv_label_set_text(label1, "count:");
    lv_obj_align_to(label1,drop_count_dec, LV_ALIGN_OUT_LEFT_MID, -5, 0);

    drop_time_dec = lv_dropdown_create(desktop_settings);//抬腿总时间
    lv_obj_add_flag(drop_time_dec, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(drop_time_dec,LV_HOR_RES/5,LV_HOR_RES/5);
    lv_dropdown_set_symbol(drop_time_dec,NULL);
    lv_obj_align_to(drop_time_dec,drop_count_dec, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_dropdown_set_options(drop_time_dec, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");//ʮλ

    drop_time_dig = lv_dropdown_create(desktop_settings);
    lv_obj_add_flag(drop_time_dig, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(drop_time_dig,LV_HOR_RES/5,LV_HOR_RES/5);
    lv_dropdown_set_symbol(drop_time_dig,NULL);
    lv_obj_align_to(drop_time_dig,drop_time_dec, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_dropdown_set_options(drop_time_dig, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");//��λ

    lv_obj_t * label2 = lv_label_create(desktop_settings);
    lv_obj_set_style_text_font(label2, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label2, "时间:");
    //lv_label_set_text(label2, "time(min):");
    lv_obj_align_to(label2,drop_time_dec, LV_ALIGN_OUT_LEFT_MID, -5, 0);

    drop_angle_dec = lv_dropdown_create(desktop_settings);//̧�ȷ���
    lv_obj_add_flag(drop_angle_dec, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(drop_angle_dec,LV_HOR_RES/5,LV_HOR_RES/5);
    lv_dropdown_set_symbol(drop_angle_dec,NULL);
    lv_obj_align_to(drop_angle_dec,drop_time_dec, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_dropdown_set_options(drop_angle_dec, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");//ʮλ

    drop_angle_dig = lv_dropdown_create(desktop_settings);
    lv_obj_add_flag(drop_angle_dig, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_align_to(drop_angle_dig,drop_angle_dec, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_set_size(drop_angle_dig,LV_HOR_RES/5,LV_HOR_RES/5);
    lv_dropdown_set_symbol(drop_angle_dig,NULL);
    lv_dropdown_set_options(drop_angle_dig, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");//��λ

    lv_obj_t * label3 = lv_label_create(desktop_settings);
    lv_obj_set_style_text_font(label3, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label3, "角度:");
    //lv_label_set_text(label3, "angle:");
    lv_obj_align_to(label3,drop_angle_dec, LV_ALIGN_OUT_LEFT_MID, -5, 0);
#endif // 0
#if 0
    //��spinbox  �������������Ӽ�����ѡ��ÿһλ�����Ǹо���������
    spinbox1 = lv_spinbox_create(desktop_settings);
    lv_obj_align(spinbox1, LV_ALIGN_CENTER, 0,0);

    lv_obj_t * img1 = lv_img_create(desktop_settings);
    lv_img_set_src(img1, LV_SYMBOL_MINUS);
    lv_obj_align_to(img1, spinbox1, LV_ALIGN_OUT_LEFT_MID, -20, 0);

    lv_obj_t * img2 = lv_img_create(desktop_settings);
    lv_img_set_src(img2, LV_SYMBOL_PLUS);
    lv_obj_align_to(img2, spinbox1, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
#endif

    lv_obj_t *btn_start = lv_btn_create(desktop_settings);
    lv_obj_set_size(btn_start,LV_HOR_RES/2,LV_HOR_RES/4);
    lv_obj_align_to(btn_start, drop_angle_dec, LV_ALIGN_OUT_BOTTOM_MID,0,20);

    lv_obj_t *label_start = lv_label_create(btn_start);
    lv_obj_align(label_start,LV_ALIGN_CENTER,0,0);
    
    //lv_label_set_text(label_start,"start");
    lv_obj_set_style_text_font(label_start, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label_start, "开始");
    lv_obj_add_event_cb(btn_start,btn_start_toggle_event_cb, LV_EVENT_PRESSED, NULL);

    bar_step = lv_bar_create(desktop_settings);//动画显示进度
    lv_obj_set_size(bar_step, LV_VER_RES,LV_HOR_RES/8);
    lv_obj_align_to(bar_step,btn_start,LV_ALIGN_OUT_TOP_MID, 0, -10);
    lv_obj_add_flag(bar_step, LV_OBJ_FLAG_HIDDEN);
    //lv_bar_set_value(bar_step 100, LV_ANIM_ON);
#if 0
    lv_obj_t *btn_back = lv_btn_create(desktop_settings);
    lv_obj_set_size(btn_back,LV_HOR_RES/2,LV_HOR_RES/4);
    lv_obj_align_to(btn_back, btn_start, LV_ALIGN_OUT_BOTTOM_MID,0,20);

    lv_obj_t *label_back = lv_label_create(btn_back);
    lv_obj_align(label_back,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(label_back,"back");
    lv_obj_add_event_cb(btn_back,btn_back_toggle_event_cb, LV_EVENT_PRESSED, NULL);
#endif

}
static void obj_bar_anim_exec_callback(void * bar, int32_t value)
{
    if (bar != NULL)
    {
        lv_bar_set_value((lv_obj_t *)bar, test_count*10, LV_ANIM_ON); //这里应该根据训练的实时步数来  
    }
}
static bool speak_end = false;
static void speak_task(void *pvParameter)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        #if 0
        char *data =NULL;
        int rxBytes = uart_read_bytes(UART_NUM_2, (uint8_t *)data, 46, 400 / portTICK_RATE_MS);
        if (rxBytes > 0) 
        {
                printf("---------------------------------------\n");
            printf("data is \n");
            for(int i = 0; i < rxBytes;i++)
            printf("  %x  ",*data++);
                printf("---------------------------------------\n");
        }
        #endif
        if(speak_end ==  false)
        {
        printf("i'm speaking\n");
        speak_command(PLEASE_CMD,NULL);
        vTaskDelay(pdMS_TO_TICKS(3300));
            //vTaskDelay(pdMS_TO_TICKS(1000));
        speak_command(LEG_ANGLE,angle_per);
         //       vTaskDelay(pdMS_TO_TICKS(1000));
                vTaskDelay(pdMS_TO_TICKS(500));
        speak_command(LEG_COUNT,count_sum);
         //       vTaskDelay(pdMS_TO_TICKS(1000));
                vTaskDelay(pdMS_TO_TICKS(500));
        speak_command(LEG_TIME,time_sum/60);
            vTaskDelay(pdMS_TO_TICKS(500));
        speak_command(START_CMD,NULL);
         //       vTaskDelay(pdMS_TO_TICKS(1000));
               // vTaskDelay(pdMS_TO_TICKS(1000));
            speak_end =true;
        }
        else{
                    test_start_flag = true;
                    vTaskSuspend(NULL);
        }
    }
     vTaskDelete(NULL);
}
static void btn_start_toggle_event_cb(lv_event_t * e)
{
    lv_anim_t anim;
    uint16_t count_dec = lv_dropdown_get_selected(drop_count_dec);
    uint16_t count_dig = lv_dropdown_get_selected(drop_count_dig);
    uint16_t angle_dec = lv_dropdown_get_selected(drop_angle_dec);
    uint16_t angle_dig = lv_dropdown_get_selected(drop_angle_dig);
    uint16_t time_dec = lv_dropdown_get_selected(drop_time_dec);
    uint16_t time_dig = lv_dropdown_get_selected(drop_time_dig);

    printf("i get num\n");
    count_sum = (uint8_t)count_dec*10+(uint8_t)count_dig;
    angle_per = (uint8_t)angle_dec*10+(uint8_t)angle_dig;
    time_sum = (uint32_t)time_dec*10+(uint32_t)time_dig;
    time_sum = time_sum*60;//转换成s
    if((count_sum == 0)||(angle_per == 0)||(time_sum == 0))
    {
        count_sum = 10;//有个默认参数  保证为0的时候
        angle_per = 80;
        time_sum = 180;//预设参数
    }
    printf("i really get num\n");
    lv_obj_clear_flag(bar_step, LV_OBJ_FLAG_HIDDEN);
    lv_anim_init(&anim); // 初始化动画
    lv_anim_set_exec_cb(&anim, obj_bar_anim_exec_callback); // 添加回调函数
    lv_anim_set_time(&anim, time_sum*1000); // 设置动画时长
    lv_anim_set_var(&anim, bar_step); // 动画绑定对象
    lv_anim_set_values(&anim, 0, 100); //设置开始值和结束值
    lv_anim_set_repeat_count(&anim, 1); // 重复次数，默认值为1 LV_ANIM_REPEAT_INFINIT用于无限重复
    lv_anim_start(&anim); // 应用动画效果

    printf("now i want to speak\n---------------------------------");
    printf("now i want to start\n---------------------------------");
                    //这个时候播放是不能一直播放的，就第一次播放
    printf("---------------------------------------\n");
    printf("i want to send angle_per---------------%d\n",angle_per);
    printf("i want to send count_sum-----------%d\n",count_sum);
    printf("i want to send time_sum------------%d\n",time_sum);
    printf("---------------------------------------\n");
    xTaskCreatePinnedToCore(speak_task, "speak", 2048, NULL, 3, NULL, 1);
}


static void page_records(void)
{
     uint8_t i;
    if((count_sum == 0)||(angle_per == 0)||(time_sum == 0))
    {
        count_sum = 10;//有个默认参数  保证为0的时候
        angle_per = 80;
        time_sum = 180;//预设参数
    }

    desktop_records = lv_obj_create(lv_scr_act());
    lv_obj_set_size(desktop_records, LV_HOR_RES, LV_VER_RES-tab_h);
    lv_obj_align(desktop_records, LV_ALIGN_BOTTOM_MID, 0,0);

    lv_obj_t *btn_show_freq = lv_btn_create(desktop_records);
    lv_obj_set_size(btn_show_freq,LV_HOR_RES,LV_HOR_RES/4);
    lv_obj_align(btn_show_freq, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *label_show_freq = lv_label_create(btn_show_freq);
    lv_obj_align(label_show_freq,LV_ALIGN_CENTER,0,0);
    //lv_obj_set_width(label_show_freq,LV_HOR_RES/2);
    //lv_obj_add_flag(label_show_freq,LV_OBJ_FLAG_SCROLLABLE);
    //lv_label_set_long_mode(label_show_freq,LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_font(label_show_freq, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    //lv_label_set_text(label_start, "开始");
    lv_label_set_text_fmt(label_show_freq,"频率:%.1fs/次",test_results.leg_freq);//��ʾƵ��

    lv_obj_t *btn_show_time = lv_btn_create(desktop_records);
    lv_obj_set_size(btn_show_time,LV_HOR_RES,LV_HOR_RES/4);
    lv_obj_align_to(btn_show_time,btn_show_freq, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    lv_obj_t *label_show_time = lv_label_create(btn_show_time);
    lv_obj_align(label_show_time,LV_ALIGN_CENTER,0,0);
    //lv_obj_set_width(label_show_time,LV_HOR_RES/2);
    //lv_label_set_long_mode(label_show_time,LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_font(label_show_time, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text_fmt(label_show_time,"总时间:%ds",test_results.test_time_sum);//��ʾƵ��
#if 1
    /*  ��ʾ̧�ȷ���*/
    lv_obj_t* chart = lv_chart_create(desktop_records); // ����Chart����
    lv_obj_align(chart,LV_ALIGN_CENTER,0,0);
    lv_obj_set_size(chart, 130, 124);  // 
    lv_chart_set_point_count(chart,count_sum);//固定当前数量  如果数目大于15，则一直refresh，count_sum

    lv_obj_align_to(chart,btn_show_time, LV_ALIGN_OUT_BOTTOM_MID, 20, 20);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/

    lv_obj_t *label_show_angle = lv_label_create(desktop_records);
    lv_obj_set_style_text_font(label_show_angle, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label_show_angle,"角度(°)");
    //lv_obj_set_size(label_show_angle,30,15);
    lv_obj_align_to(label_show_angle,chart, LV_ALIGN_OUT_LEFT_TOP, 40, 20);

    /*Add two data series*/
    lv_chart_series_t* ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y); // ������1��chart series,���ú�ɫ
    lv_chart_series_t* ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_SECONDARY_Y); // ������1��chart series,������ɫ
    /*Set the next points on 'ser1'*/
    lv_obj_set_style_size(chart,            // 图表对像
                          4,                 // 点的size大小
                          LV_PART_INDICATOR);// 指示器
    //lv_chart_set_zoom_x(chart, 512);
    lv_chart_set_all_value(chart,ser1,(lv_coord_t)angle_per);
    /*Directly set points on 'ser2'*/
    for(i = 0;i < count_sum ;i++)
    lv_chart_set_next_value(chart,ser2,(lv_coord_t)((test_results.lv_test_angle[i])*5/8));

    lv_chart_set_range(chart,LV_CHART_AXIS_PRIMARY_Y,0,160);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 5, 2,3 ,2, true, 40); // y��tick��ʾ,��ʾ3���̶�ֵ
    lv_chart_refresh(chart); /*Required after direct set*/

    lv_obj_t *btn_chart = lv_btn_create(chart);//按键需要根据图的宽度来设置位置
    lv_obj_align(btn_chart,LV_ALIGN_BOTTOM_MID,0,-10);
    lv_obj_set_size(btn_chart,1,1);
    //lv_obj_add_event_cb(btn_chart, obj_btn_chart_exec_callback, LV_EVENT_PRESSED|LV_EVENT_FOCUSED, NULL);
    //lv_chart_refresh(chart);
    //lv_obj_t *btn_chart3 = lv_btn_create(chart);//按键需要根据图的宽度来设置位置
    //lv_obj_align(btn_chart3,LV_ALIGN_BOTTOM_MID,80,-20);
    //lv_obj_set_size(btn_chart3,1,1);
    #endif
#if 1
    /*  ��ʾ̧��ʱ��/����*/
    lv_obj_t* chart2 = lv_chart_create(desktop_records); // ����Chart����
    lv_obj_set_size(chart2, 130, 124);  // ���ô�С
    lv_chart_set_point_count(chart2,count_sum);//15�ǱȽϺõ�Ч���ˣ����Ҫ��������ټ�һ���жϣ��������ص���ȥ

    //lv_obj_add_flag(chart2,LV_OBJ_FLAG_FLOATING);
    lv_obj_align_to(chart2,chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_chart_set_type(chart2, LV_CHART_TYPE_LINE); /*Show lines and points too*/

    lv_obj_t *label_show_per_time = lv_label_create(desktop_records);
    lv_obj_set_style_text_font(label_show_per_time, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label_show_per_time,"时间(s)");
    //lv_obj_set_size(label_show_angle,30,15);
    lv_obj_align_to(label_show_per_time,chart2, LV_ALIGN_OUT_LEFT_TOP, 40, 20);
    
    lv_chart_series_t* ser3 = lv_chart_add_series(chart2, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y); // ������1��chart series,���ú�ɫ
    lv_chart_series_t* ser4 = lv_chart_add_series(chart2, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_SECONDARY_Y); // ������1��chart series,������ɫ
    lv_obj_set_style_size(chart2,            // 图表对像
                          4,                 // 点的size大小
                          LV_PART_INDICATOR);// 指示器
    //lv_chart_set_zoom_x(chart, 512);
    lv_chart_set_all_value(chart2,ser3,(lv_coord_t)time_sum/count_sum);//预设频率  s/次
    /*Set the next points on 'ser1'*/
    for(i = 0;i < count_sum ;i++)
    lv_chart_set_next_value(chart2,ser4,(lv_coord_t)((test_results.lv_test_time[i])*5/3));
    lv_chart_set_range(chart2,LV_CHART_AXIS_PRIMARY_Y,0,60);
    /*Directly set points on 'ser2'*/
    //lv_chart_set_ext_y_array(chart2,ser4,array);

    lv_chart_set_axis_tick(chart2, LV_CHART_AXIS_PRIMARY_Y, 6, 2,3 ,2, true, 40); // y��tick��ʾ,��ʾ3���̶�ֵ
    lv_chart_refresh(chart2); /*Required after direct set*/

    lv_obj_t *btn_chart2 = lv_btn_create(chart2);
    lv_obj_align(btn_chart2,LV_ALIGN_BOTTOM_MID,0,0);
    lv_obj_set_size(btn_chart2,1,1);
#endif 
}

static void page_voice(void)
{
    desktop_voice = lv_obj_create(lv_scr_act());
#if 1
    lv_obj_set_size(desktop_voice, LV_HOR_RES, LV_VER_RES-40);
    lv_obj_align(desktop_voice, LV_ALIGN_BOTTOM_MID, 0,0);

    lv_obj_t *drop_voice = lv_dropdown_create(desktop_voice);//��������
    lv_obj_align(drop_voice, LV_ALIGN_RIGHT_MID, 0,0);
    lv_obj_set_size(drop_voice,LV_HOR_RES/2,LV_HOR_RES/4);
    lv_dropdown_set_options(drop_voice, "Medim\nLarge\nQuiet\nSmall\n");

    lv_obj_t * label1 = lv_label_create(desktop_voice);
    lv_obj_set_style_text_font(label1, &lv_font_source_han_sans_bold_28, 0);   // 使用自定义的字库
    lv_label_set_text(label1, "声音:");
    lv_obj_align_to(label1,drop_voice, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    #endif
/*加个event事件*/
#if 0
            char buf[6] = {0};
        char prv_buf[6] = {0};
        strncpy(prv_buf,buf,6);
        lv_dropdown_get_selected_str(drop_voice,(char *)buf,6);
        if(strncmp(prv_buf,buf,6) != 0)
        {
            if(strncmp(buf,"Quiet",6)==0)
            speak_command(SET_VOLUME,0);
            else  if(strncmp(buf,"Small",6)==0)
            speak_command(SET_VOLUME,10);
            else  if(strncmp(buf,"Large",6)==0)
            speak_command(SET_VOLUME,30);
            else 
            speak_command(SET_VOLUME,20);
            printf("volume %s\n",buf);
            strncpy(prv_buf,buf,6);
        }
    
    #endif

}
/**********************
 *   STATIC FUNCTIONS
 **********************/
static void button_Task(void *pvParameter) 
{
    while(1)
    {
        if(gpio_get_level(KEY_BACK)==1)
        {
            printf("key_back\r\n");
            vTaskDelay(100);//长按
            if(gpio_get_level(KEY_BACK)==1)
            {
                printf("key_back_i will delete\r\n");
                switch(flag)
                {
#if HIDE_WIN
                case 1:
                        lv_obj_clear_flag(main_desktop, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_add_flag(desktop_settings, LV_OBJ_FLAG_HIDDEN);
                    break;
                case 2:
                        lv_obj_clear_flag(main_desktop, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_add_flag(desktop_records, LV_OBJ_FLAG_HIDDEN);
                case 3:
                        lv_obj_clear_flag(main_desktop, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_add_flag(desktop_voice, LV_OBJ_FLAG_HIDDEN);
                        break;
#else
                case 1:lv_obj_del(desktop_settings);main_page();break;
                case 2:lv_obj_del(desktop_records);main_page();break;
                case 3:lv_obj_del(desktop_voice);main_page();break;
#endif
                default:break;
                }
              flag = 0;  
            }                  
        }
        vTaskDelay(200);//长按
    }
     vTaskDelete(NULL);
}



#endif
