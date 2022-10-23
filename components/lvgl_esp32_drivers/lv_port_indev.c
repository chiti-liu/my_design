/**
 * @file lv_port_indev_templ.c
 *
 */

 /*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"
#include "lvgl.h"
#include "driver/gpio.h"
#include <stdio.h>
#define USE_VERTICAL 1//横向显示
#define USE_VERTICAL_REV 0//横向反向

#if USE_VERTICAL
#define KEY_UP 34
#define KEY_DOWN 36
#define KEY_LEFT 35
#define KEY_RIGHT 39
#define KEY_ENTER 2

#elif USE_VERTICAL_REV
#define KEY_UP 36
#define KEY_DOWN 34
#define KEY_LEFT 39
#define KEY_RIGHT 35
#define KEY_ENTER 2
#else //垂直显示
#define KEY_UP 39
#define KEY_DOWN 35
#define KEY_LEFT 36
#define KEY_RIGHT 34
#define KEY_ENTER 2
#endif

// static void button_init(void);
// static bool button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
// static int8_t button_get_pressed_id(void);
// static bool button_is_pressed(uint8_t id);

/**********************
 *  STATIC VARIABLES
 **********************/



/**********************
 *      MACROS
 **********************/
/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static uint8_t keypad_get_key(void)
{
    uint8_t value=0;
    /*Your code comes here*/
    if(gpio_get_level(KEY_UP)==1)
       {value = 2;printf("key_up");}
    else if(gpio_get_level(KEY_DOWN)==1)
        {value = 1;printf("key down");}
    else if(gpio_get_level(KEY_LEFT)==1)
        value = 3;
    else if(gpio_get_level(KEY_RIGHT)==1)
        value = 4;
    else if(gpio_get_level(KEY_ENTER)==1)
        {value = 5;printf("key_enter");}
    else 
        value = 0;

    return value;
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static uint8_t last_key = 0;
    uint8_t value = keypad_get_key();
    /*Get the current x and y coordinates*/
    //mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the a key is pressed and save the pressed key*/
    uint8_t act_key = value;
    if(act_key != 0) {
        data->state = LV_INDEV_STATE_PR;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch(act_key) {
        case 1:
            act_key = LV_KEY_NEXT;
            break;
        case 2:
            act_key = LV_KEY_PREV;
            break;
        case 3:
            act_key = LV_KEY_LEFT;
            break;
        case 4:
            act_key = LV_KEY_RIGHT;
            break;
        case 5:
            act_key = LV_KEY_ENTER;
            break;
        }
        last_key = act_key;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->key = last_key;

}
lv_indev_t * indev_keypad;
void lv_port_indev_init(void)
{

    static lv_indev_drv_t indev_drv;
    /*Initialize your encoder if you have*/
    /*Register a encoder input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv);
}
#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;
#endif
