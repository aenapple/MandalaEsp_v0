#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "lvgl.h"
#include "panel_st7701.h"
#include "ui.h"

#define LCD_H_RES 480
#define LCD_V_RES 480

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = NULL;
static esp_lcd_panel_handle_t rgb_panel = NULL;

static void lvgl_flush_cb(lv_disp_drv_t *drv,
                          const lv_area_t *area,
                          lv_color_t *color_p)
{
    esp_lcd_panel_handle_t panel = drv->user_data;
    esp_lcd_panel_draw_bitmap(panel,
                              area->x1, area->y1,
                              area->x2 + 1, area->y2 + 1,
                              color_p);
    lv_disp_flush_ready(drv);
}

static void lv_tick_task(void *arg)
{
    lv_tick_inc(5);
}

static void init_sdcard(void)
{
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
    };

    sdmmc_card_t *card;
    esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
}

static void init_lvgl(void)
{
    lv_init();

    buf1 = heap_caps_malloc(LCD_H_RES * 40 * sizeof(lv_color_t),
                            MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM);
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, LCD_H_RES * 40);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.user_data = rgb_panel;
    lv_disp_drv_register(&disp_drv);

    const esp_timer_create_args_t tick_args = {
        .callback = lv_tick_task,
        .name = "lv_tick"
    };
    esp_timer_handle_t tick_timer;
    esp_timer_create(&tick_args, &tick_timer);
    esp_timer_start_periodic(tick_timer, 5000);
}

void app_main(void)
{
    rgb_panel = panel_st7701_init();
    init_sdcard();
    init_lvgl();
    ui_init();

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

