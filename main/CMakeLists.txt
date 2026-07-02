idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        mpu6050
        gesture
        oled_ui
        button
        command
        json_builder
        uart_sender
        wifi_manager
        mqtt_sender
)