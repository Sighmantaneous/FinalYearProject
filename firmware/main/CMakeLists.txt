idf_component_register(SRCS "main.c" "radio_con_test.c"
                       REQUIRES mesh
                       PRIV_REQUIRES esp_driver_spi esp_driver_gpio esp_timer
                       INCLUDE_DIRS "")
