#include "ota_manager.h"
#include "esp_log.h"
#include "esp_ota_ops.h"


static const char  *TAG="ota_manager";

esp_err_t ota_manager_print_partition_info(void)
{
    const esp_partition_t *running=esp_ota_get_running_partition();

    if(running==NULL)
    {
        ESP_LOGE(TAG,"Failed to  get running partition");
        return  ESP_FAIL;
    }
    ESP_LOGI(TAG,
              "Running partition :label=%s address=0x%lx size=%lu",
              running->label,
              (unsigned long)running->address,
              (unsigned long)running->size 
                );

    const esp_partition_t *next=esp_ota_get_next_update_partition(NULL);

    if(next==NULL)
    {
        ESP_LOGE(TAG,"Fail to get next OTA partition");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG,
            "Next  OTA partition:label =%s  address=0x%lx size=%lu ",
            next->label,
            (unsigned long) next->address,
            (unsigned long)next->size
    );
    return ESP_OK;  
}