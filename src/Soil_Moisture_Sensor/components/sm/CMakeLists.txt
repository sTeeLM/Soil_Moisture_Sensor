idf_component_register(
                    SRCS "sm.c" "state_machines/sm_main.c"
                    INCLUDE_DIRS "include" "state_machines"
                    REQUIRES "task"
                    REQUIRES "logger"
                    REQUIRES "power"
                    REQUIRES "oled"
                    REQUIRES "clock"
                    REQUIRES "hygrometer"
                    REQUIRES "mini_font"
                    REQUIRES "ble_reporter")
