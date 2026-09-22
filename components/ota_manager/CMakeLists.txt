idf_component_register(
    SRCS
        "ota_writer.c"
        "ota_manager.c"
        "ota_downloader.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        app_update
        esp_http_client
        mbedtls
)
