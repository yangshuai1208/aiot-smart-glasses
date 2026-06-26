#include "button.h"

#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUTTON_GPIO     GPIO_NUM_4
#define BUTTON_ACTIVE_LEVEL   0
#define SHORT_PRESS_MIN_MS    50
#define LONG_PRESS_MS          1000

static TickType_t press_start_tick=0;
static int last_level=1;
static int is_pressed=0;
static int long_event_sent=0;

esp_err_t button_init(void)
{
    gpio_config_t io_conf={
        .pin_bit_mask=(1ULL<<BUTTON_GPIO),
        .mode=GPIO_MODE_INPUT,
        .pull_up_en=GPIO_PULLUP_ENABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .intr_type=GPIO_INTR_DISABLE,
    };
    return gpio_config(&io_conf);
}
button_event_t button_get_event(void)
{
    int  level=gpio_get_level(BUTTON_GPIO);
    TickType_t now=xTaskGetTickCount();

    if(level==BUTTON_ACTIVE_LEVEL&&last_level!=BUTTON_ACTIVE_LEVEL){
        is_pressed=1;
        long_event_sent=0;
        press_start_tick=now;
    }
    if(level==BUTTON_ACTIVE_LEVEL&&is_pressed){
        uint32_t press_ms=(now-press_start_tick)*portTICK_PERIOD_MS;
        
        if(press_ms>=LONG_PRESS_MS&&long_event_sent==0){
            long_event_sent=1;
            last_level=level;
            return BUTTON_EVENT_LONG_PRESS;
        }
    }
    if(level!=BUTTON_ACTIVE_LEVEL&&last_level==BUTTON_ACTIVE_LEVEL){
        if(is_pressed)
        {
            uint32_t press_ms=(now-press_start_tick)*portTICK_PERIOD_MS;

            is_pressed=0;

            if(press_ms>=SHORT_PRESS_MIN_MS&&press_ms<LONG_PRESS_MS){
                last_level=level;
                return BUTTON_EVENT_SHORT_PRESS;
            }
        }
    }
    last_level=level;
    return BUTTON_EVENT_NONE;
}