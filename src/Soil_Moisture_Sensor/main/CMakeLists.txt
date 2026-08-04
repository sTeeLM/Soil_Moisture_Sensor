idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES logger mini_font oled delay gpio_wrapper i2c_wrapper sm task power key hygrometer adc_wrapper ble_reporter)