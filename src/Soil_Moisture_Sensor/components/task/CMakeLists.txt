idf_component_register(SRCS "task.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "sm"
                    REQUIRES "clock"
                    REQUIRES "key"
                    REQUIRES "power"
                    REQUIRES "logger")
