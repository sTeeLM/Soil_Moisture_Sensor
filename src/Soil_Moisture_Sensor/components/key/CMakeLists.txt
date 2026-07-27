idf_component_register(SRCS "key.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "logger"
                    REQUIRES "task"
                    REQUIRES "gpio_wrapper"
                    REQUIRES "clock"
                    REQUIRES "sm")
