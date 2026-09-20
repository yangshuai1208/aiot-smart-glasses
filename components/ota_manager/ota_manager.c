#include "ota_manager.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include <stdbool.h>

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
esp_err_t ota_manager_check_and_confirm_app(void)
{
    const esp_partition_t *running=esp_ota_get_running_partition();

    if(running==NULL)
    {
        ESP_LOGE(TAG,"Failed to get running partition");
        return ESP_FAIL;
    }
    esp_ota_img_states_t ota_state;

    esp_err_t ret=esp_ota_get_state_partition(running,&ota_state);

    if(ret!=ESP_OK)
    {
        ESP_LOGW(TAG,"Cannot get OTA state:%s",esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG,"Running partition=%s,state=%d",running->label,(int)ota_state);

    if(ota_state==ESP_OTA_IMG_PENDING_VERIFY)
    {
        ESP_LOGI(TAG,"New firware pending verification");
        bool self_test_ok=true;
        
        if(self_test_ok)
        {
            ESP_LOGI(TAG,"Self-test passed,mark app valid");

            return esp_ota_mark_app_valid_cancel_rollback();
        }
        else 
        {
            ESP_LOGE(TAG,"Self-test failed,rollback");

            return esp_ota_mark_app_invalid_rollback_and_reboot();
        }
    }
    ESP_LOGI(TAG,"Current firmware  does not require verification ");

    return ESP_OK;

}