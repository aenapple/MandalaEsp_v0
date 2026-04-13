idf_component_register(
        SRCS "main.c" "panel_st7701.c" "ui.c"
        INCLUDE_DIRS "."
        REQUIRES esp_lcd lvgl fatfs freertos
)