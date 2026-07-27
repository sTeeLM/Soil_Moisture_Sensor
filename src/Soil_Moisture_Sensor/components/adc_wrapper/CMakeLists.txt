idf_component_register(SRCS "adc_wrapper.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "logger"
                    REQUIRES "esp_adc")