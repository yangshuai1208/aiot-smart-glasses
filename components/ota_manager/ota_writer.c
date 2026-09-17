#include "ota_writer.h"

#include "esp_ota_ops.h"
#include "esp_log.h"

static const char *TAG="ota_writer";

static esp_ota_handle_t ota_handle;

static const esp_partition_t *update_partition;


esp_err_t ota_writer_begin(void)
{
    update_partition=esp_ota_get_next_update_partition(NULL);


    if(update_partition==NULL)
    {
        ESP_LOGE(TAG,"No OTA partition");
        return ESP_FAIL;
    }

    return esp_ota_begin(update_partition,
                        OTA_SIZE_UNKNOWN,
                        &ota_handle);
}
esp_err_t ota_writer_write(
        const uint8_t *data,
        size_t len)
{
    if(data == NULL || len == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }


    return esp_ota_write(
            ota_handle,
            data,
            len);
}


esp_err_t ota_writer_finish(void)
{
    esp_err_t ret;


    ret=esp_ota_end(ota_handle);

    if(ret!=ESP_OK)
    {
        ESP_LOGE(TAG,
                    "OTA end failed");

        return ret;
    }
    ret=esp_ota_set_boot_partition(update_partition);

    if(ret!=ESP_OK)
    {
        ESP_LOGE(TAG,"Set boot partition failed");
    }
    return ret;
}
