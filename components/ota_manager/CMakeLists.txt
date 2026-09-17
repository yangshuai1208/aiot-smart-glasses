idf_component_register(
                        SRCS "ota_writer.c" "ota_manager.c"
                        INCLUDE_DIRS "include"
                        REQUIRES app_update
)