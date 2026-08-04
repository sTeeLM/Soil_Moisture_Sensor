idf_component_register(SRCS "oled_ext.c" "oled.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "i2c_wrapper"
                    REQUIRES "gpio_wrapper"
                    REQUIRES "delay"
                    REQUIRES "mini_font"
                    REQUIRES "logger")
