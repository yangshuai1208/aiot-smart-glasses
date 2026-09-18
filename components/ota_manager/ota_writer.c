#include "ota_writer.h"

#include "esp_ota_ops.h"
#include "esp_log.h"

#include <stdbool.h>

static const char *TAG="ota_writer";

static esp_ota_handle_t ota_handle;

static const esp_partition_t *update_partition=NULL;

static bool ota_started=false;

esp_err_t ota_writer_begin(void)
{

    if(ota_started)
    {
        ESP_LOGW(TAG,"OTA  already  started");
        return ESP_ERR_INVALID_STATE;
    }

    update_partition=esp_ota_get_next_update_partition(NULL);


    if(update_partition==NULL)
    {
        ESP_LOGE(TAG,"No OTA partition");
        return ESP_FAIL;
    }

 

    esp_err_t ret=esp_ota_begin(update_partition,
                        OTA_SIZE_UNKNOWN,
                        &ota_handle);

    if(ret!=ESP_OK)
    {
        ESP_LOGE(
                TAG,
                "esp_ota_begin failed:%s",
                esp_err_to_name(ret));
        return ret;
    }
    ota_started=true;

    ESP_LOGI(TAG,
            "OTA started, partition=%s",
            update_partition->label);

    return ESP_OK;

}
esp_err_t ota_writer_write(
        const uint8_t *data,
        size_t len)
{
    if(!ota_started)
    {
        return ESP_ERR_INVALID_STATE;
    }


    if(data == NULL || len == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }


    esp_err_t ret= esp_ota_write(
            ota_handle,
            data,
            len);

    if(ret!=ESP_OK)
    {
        ESP_LOGE(TAG,
                "esp_ota_write failed:%s",
                esp_err_to_name(ret));
    }
    return ret;
}


esp_err_t ota_writer_finish(void)
{

    if(!ota_started)
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret=esp_ota_end(ota_handle);

    ota_started=false;

    
    if(ret!=ESP_OK)
    {
        ESP_LOGE(TAG,
                    "esp_ota_end failed :%s",
                    esp_err_to_name(ret));

        return ret;
    }
    ret=esp_ota_set_boot_partition(update_partition);

    if(ret!=ESP_OK)
    {
        ESP_LOGE(TAG,"Set boot partition failed:%s",esp_err_to_name(ret));
        
        return ret;
    }
    ESP_LOGI(TAG,"OTA finished ,next partition =%s",update_partition->label);
    return ESP_OK;
}
esp_err_t  ota_writer_abort(void)
{
    if(!ota_started)
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret=esp_ota_abort(ota_handle);

    ota_started=false;
    ota_handle=0;
    update_partition=NULL;

    if(ret!=ESP_OK)
    {
        ESP_LOGE(TAG,
                "OTA aborted failed:%s",
                 esp_err_to_name(ret));
        
        return ret;
    }

    ESP_LOGW(TAG,"OTA aborted");

    return ESP_OK;

}