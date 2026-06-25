#include "oled_ui.h"

#include <string.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define OLED_ADDR    0x3C
#define OLED_WIDTH   128
#define OLED_PAGES   8
#define I2C_MASTER_NUM I2C_NUM_0

static const char*TAG="OLED_UI";

static esp_err_t oled_write_cmd(uint8_t cmd)
{
    uint8_t data[2]={0x00,cmd};

    return i2c_master_write_to_device(I2C_MASTER_NUM,
                                     OLED_ADDR,
                                     data,
                                     sizeof(data),
                                     pdMS_TO_TICKS(1000));
}
static esp_err_t oled_write_data(const uint8_t *data,size_t len)
{
    uint8_t buf[17];

    if(len>16)
    {
        len=16;
    }
    
    buf[0]=0x40;
    memcpy(&buf[1],data,len);

    return i2c_master_write_to_device(I2C_MASTER_NUM,
                                        OLED_ADDR,
                                        buf,
                                        len+1,
                                        pdMS_TO_TICKS(1000));
}
static void oled_set_cursor(uint8_t page,uint8_t col)
{
    oled_write_cmd(0xB0+page);
    oled_write_cmd(0x00+(col&0x0F));
    oled_write_cmd(0x10+((col>>4)&0x0F));
}
static void oled_clear(void)
{
    uint8_t zeros[16]={0};

    for(uint8_t page=0;page<OLED_PAGES;page++)
    {
        oled_set_cursor(page,0);
        for(uint8_t col=0;col<OLED_WIDTH;col+=16)
        {
            oled_write_data(zeros,16);
        }
    }
}
static void font5x7_get(char c, uint8_t out[5])
{
    memset(out, 0x00, 5);

    switch (c) {
    case 'A': { uint8_t f[5] = {0x7E,0x11,0x11,0x11,0x7E}; memcpy(out,f,5); break; }
    case 'B': { uint8_t f[5] = {0x7F,0x49,0x49,0x49,0x36}; memcpy(out,f,5); break; }
    case 'C': { uint8_t f[5] = {0x3E,0x41,0x41,0x41,0x22}; memcpy(out,f,5); break; }
    case 'D': { uint8_t f[5] = {0x7F,0x41,0x41,0x22,0x1C}; memcpy(out,f,5); break; }
    case 'E': { uint8_t f[5] = {0x7F,0x49,0x49,0x49,0x41}; memcpy(out,f,5); break; }
    case 'F': { uint8_t f[5] = {0x7F,0x09,0x09,0x09,0x01}; memcpy(out,f,5); break; }
    case 'G': { uint8_t f[5] = {0x3E,0x41,0x49,0x49,0x7A}; memcpy(out,f,5); break; }
    case 'H': { uint8_t f[5] = {0x7F,0x08,0x08,0x08,0x7F}; memcpy(out,f,5); break; }
    case 'I': { uint8_t f[5] = {0x00,0x41,0x7F,0x41,0x00}; memcpy(out,f,5); break; }
    case 'J': { uint8_t f[5] = {0x20,0x40,0x41,0x3F,0x01}; memcpy(out,f,5); break; }
    case 'K': { uint8_t f[5] = {0x7F,0x08,0x14,0x22,0x41}; memcpy(out,f,5); break; }
    case 'L': { uint8_t f[5] = {0x7F,0x40,0x40,0x40,0x40}; memcpy(out,f,5); break; }
    case 'M': { uint8_t f[5] = {0x7F,0x02,0x0C,0x02,0x7F}; memcpy(out,f,5); break; }
    case 'N': { uint8_t f[5] = {0x7F,0x04,0x08,0x10,0x7F}; memcpy(out,f,5); break; }
    case 'O': { uint8_t f[5] = {0x3E,0x41,0x41,0x41,0x3E}; memcpy(out,f,5); break; }
    case 'P': { uint8_t f[5] = {0x7F,0x09,0x09,0x09,0x06}; memcpy(out,f,5); break; }
    case 'Q': { uint8_t f[5] = {0x3E,0x41,0x51,0x21,0x5E}; memcpy(out,f,5); break; }
    case 'R': { uint8_t f[5] = {0x7F,0x09,0x19,0x29,0x46}; memcpy(out,f,5); break; }
    case 'S': { uint8_t f[5] = {0x46,0x49,0x49,0x49,0x31}; memcpy(out,f,5); break; }
    case 'T': { uint8_t f[5] = {0x01,0x01,0x7F,0x01,0x01}; memcpy(out,f,5); break; }
    case 'U': { uint8_t f[5] = {0x3F,0x40,0x40,0x40,0x3F}; memcpy(out,f,5); break; }
    case 'V': { uint8_t f[5] = {0x1F,0x20,0x40,0x20,0x1F}; memcpy(out,f,5); break; }
    case 'W': { uint8_t f[5] = {0x3F,0x40,0x38,0x40,0x3F}; memcpy(out,f,5); break; }
    case 'X': { uint8_t f[5] = {0x63,0x14,0x08,0x14,0x63}; memcpy(out,f,5); break; }
    case 'Y': { uint8_t f[5] = {0x07,0x08,0x70,0x08,0x07}; memcpy(out,f,5); break; }
    case 'Z': { uint8_t f[5] = {0x61,0x51,0x49,0x45,0x43}; memcpy(out,f,5); break; }
    case '0': { uint8_t f[5] = {0x3E,0x51,0x49,0x45,0x3E}; memcpy(out,f,5); break; }
    case '1': { uint8_t f[5] = {0x00,0x42,0x7F,0x40,0x00}; memcpy(out,f,5); break; }
    case '2': { uint8_t f[5] = {0x42,0x61,0x51,0x49,0x46}; memcpy(out,f,5); break; }
    case '3': { uint8_t f[5] = {0x21,0x41,0x45,0x4B,0x31}; memcpy(out,f,5); break; }
    case '4': { uint8_t f[5] = {0x18,0x14,0x12,0x7F,0x10}; memcpy(out,f,5); break; }
    case '5': { uint8_t f[5] = {0x27,0x45,0x45,0x45,0x39}; memcpy(out,f,5); break; }
    case '6': { uint8_t f[5] = {0x3C,0x4A,0x49,0x49,0x30}; memcpy(out,f,5); break; }
    case '7': { uint8_t f[5] = {0x01,0x71,0x09,0x05,0x03}; memcpy(out,f,5); break; }
    case '8': { uint8_t f[5] = {0x36,0x49,0x49,0x49,0x36}; memcpy(out,f,5); break; }
    case '9': { uint8_t f[5] = {0x06,0x49,0x49,0x29,0x1E}; memcpy(out,f,5); break; }
    case ':': { uint8_t f[5] = {0x00,0x36,0x36,0x00,0x00}; memcpy(out,f,5); break; }
    case '-': { uint8_t f[5] = {0x08,0x08,0x08,0x08,0x08}; memcpy(out,f,5); break; }
    case ' ': default:
        break;
    }
}
static void oled_draw_char(char c)
{
    uint8_t font[5];
    uint8_t buf[6];

    font5x7_get(c,font);

    buf[0]=font[0];
    buf[1]=font[1];
    buf[2]=font[2]; 
    buf[3]=font[3];
    buf[4]=font[4];
    buf[5]=0x00;
        
    oled_write_data(buf,6);
}

static void oled_draw_string(uint8_t page,uint8_t col,const char*str)
{
    oled_set_cursor(page,col);

    while(*str!='\0'){
        oled_draw_char(*str);
        str++;
    }
}
esp_err_t oled_ui_init(void)
{
    const uint8_t init_cmds[] = {
        0xAE,
        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,
        0x8D, 0x14,
        0x20, 0x00,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0xCF,
        0xD9, 0xF1,
        0xDB, 0x40,
        0xA4,
        0xA6,
        0xAF
    };
    for(size_t i=0;i<sizeof(init_cmds);i++)
    {
        esp_err_t ret=oled_write_cmd(init_cmds[i]);
        if(ret!=ESP_OK){
            ESP_LOGE(TAG,"OLED init failed");
            return ret;
        }
    }
    oled_clear();

    ESP_LOGI(TAG,"OLED init success");

    return ESP_OK;
}
esp_err_t oled_ui_show_status(const char*mode,
                              gesture_type_t gesture,
                              const char*cmd,
                              const char*status)
{
    oled_clear();

    oled_draw_string(0,0,"MODE:");
    oled_draw_string(0,36,mode);

    oled_draw_string(2,0,"GEST:");
    oled_draw_string(2,36,gesture_to_string(gesture));

    oled_draw_string(4,0,"CMD:");
    oled_draw_string(4,30,cmd);

    oled_draw_string(6,0,"STATUS:");
    oled_draw_string(6,48,status);


    return ESP_OK;
}  