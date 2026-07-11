idf_component_register(SRCS "clock.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "logger"
                    REQUIRES "task"
                    REQUIRES "sm"
                    REQUIRES "esp_driver_gptimer")
