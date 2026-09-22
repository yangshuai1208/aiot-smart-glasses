#include "ota_downloader.h"

#include "ota_writer.h"

#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include <stdint.h>

#define OTA_DOWNLOAD_BUF_SIZE 4096

static const char *TAG =
    "ota_downloader";

static esp_err_t ota_http_open_final(
    esp_http_client_handle_t client,
    int64_t *content_length)
{
    for (int hop = 0; hop < 6; ++hop)
    {
        esp_err_t ret =
            esp_http_client_open(client, 0);

        if (ret != ESP_OK)
        {
            return ret;
        }

        int64_t len =
            esp_http_client_fetch_headers(client);

        if (len < 0)
        {
            esp_http_client_close(client);
            return ESP_FAIL;
        }

        int status =
            esp_http_client_get_status_code(client);

        ESP_LOGI(TAG, "HTTP status=%d", status);

        if (status == 200)
        {
            *content_length = len;
            return ESP_OK;
        }

        if (status == 301 ||
            status == 302 ||
            status == 303 ||
            status == 307 ||
            status == 308)
        {
            if (hop == 5)
            {
                esp_http_client_close(client);
                return ESP_FAIL;
            }

            ret =
                esp_http_client_set_redirection(client);

            esp_http_client_close(client);

            if (ret != ESP_OK)
            {
                return ret;
            }

            continue;
        }

        esp_http_client_close(client);
        return ESP_FAIL;
    }

    return ESP_FAIL;
}


esp_err_t ota_downloader_start(
    const char *url)
{
    if (url == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_config_t config =
    {
        .url = url,

      
        .crt_bundle_attach =
            esp_crt_bundle_attach,

        .timeout_ms = 10000,
    };


    esp_http_client_handle_t client =
        esp_http_client_init(
            &config);

    if (client == NULL)
    {
        ESP_LOGE(
            TAG,
            "HTTP client init failed");

        return ESP_FAIL;
    }


int64_t content_length = 0;

    esp_err_t ret =
    ota_http_open_final(
        client,
        &content_length);

    if (ret != ESP_OK)
    {
    ESP_LOGE(TAG, "HTTP download open failed");

    esp_http_client_cleanup(client);
    return ret;
    }
    ESP_LOGI(
        TAG,
        "Firmware content length=%lld",
        (long long)content_length);


    ret = ota_writer_begin();

    if (ret != ESP_OK)
    {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);

        return ret;
    }

    static uint8_t buffer[OTA_DOWNLOAD_BUF_SIZE];

    size_t total_written = 0;


    while (1)
    {
        int read_len =
            esp_http_client_read(
                client,
                (char *)buffer,
                sizeof(buffer));

  
        if (read_len > 0)
        {
            ret =
                ota_writer_write(
                    buffer,
                    (size_t)read_len);

            if (ret != ESP_OK)
            {
                ESP_LOGE(
                    TAG,
                    "OTA write failed");

                ota_writer_abort();

                esp_http_client_close(
                    client);

                esp_http_client_cleanup(
                    client);

                return ret;
            }

            total_written +=
                (size_t)read_len;

            ESP_LOGI(
                TAG,
                "OTA downloaded=%u bytes",
                (unsigned)total_written);

            continue;
        }


  
        if (read_len == 0)
        {
            break;
        }


        ESP_LOGE(
            TAG,
            "HTTP read failed");

        ota_writer_abort();

        esp_http_client_close(
            client);

        esp_http_client_cleanup(
            client);

        return ESP_FAIL;
    }



    if (!esp_http_client_is_complete_data_received(
            client))
    {
        ESP_LOGE(
            TAG,
            "Incomplete firmware download");

        ota_writer_abort();

        esp_http_client_close(
            client);

        esp_http_client_cleanup(
            client);

        return ESP_FAIL;
    }


    ret = ota_writer_finish();

    if (ret != ESP_OK)
    {
        esp_http_client_close(
            client);

        esp_http_client_cleanup(
            client);

        return ret;
    }


    ESP_LOGI(
        TAG,
        "OTA download completed, bytes=%u",
        (unsigned)total_written);


    esp_http_client_close(
        client);

    esp_http_client_cleanup(
        client);

    return ESP_OK;
}