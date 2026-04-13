#include "panel_st7701.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ST7701";

#define LCD_H_RES 480
#define LCD_V_RES 480

// SPI → ST7701
#define PIN_NUM_LCD_CS    6
#define PIN_NUM_LCD_SCK   7
#define PIN_NUM_LCD_MOSI  8
#define PIN_NUM_LCD_RST   5

// RGB → матрица
#define PIN_NUM_PCLK   9
#define PIN_NUM_DE     46
#define PIN_NUM_VSYNC  3
#define PIN_NUM_HSYNC  4

// RGB565: B0..B4, G0..G5, R0..R4
static int rgb_pins[] = {
    14, 21, 47, 48, 45,      // B0..B4
    38, 39, 40, 41, 42, 2,   // G0..G5
    1, 10, 11, 12, 13        // R0..R4
};

static void st7701_write_cmd(esp_lcd_panel_io_handle_t io, uint8_t cmd)
{
    esp_lcd_panel_io_tx_param(io, cmd, NULL, 0);
}

static void st7701_write_data(esp_lcd_panel_io_handle_t io, uint8_t cmd, const uint8_t *data, size_t len)
{
    esp_lcd_panel_io_tx_param(io, cmd, data, len);
}

static void st7701_init_sequence(esp_lcd_panel_io_handle_t io)
{
    st7701_write_cmd(io, 0x11);
    vTaskDelay(pdMS_TO_TICKS(120));

    uint8_t fmt = 0x55; // RGB565
    st7701_write_data(io, 0x3A, &fmt, 1);

    st7701_write_cmd(io, 0x21); // inversion on
    st7701_write_cmd(io, 0x29); // display on
    vTaskDelay(pdMS_TO_TICKS(20));
}

esp_lcd_panel_handle_t panel_st7701_init(void)
{
    ESP_LOGI(TAG, "Init SPI bus for ST7701");
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_LCD_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_NUM_LCD_SCK,
        .max_transfer_sz = 4096,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = -1,
        .cs_gpio_num = PIN_NUM_LCD_CS,
        .pclk_hz = 40 * 1000 * 1000,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };

    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle));

    gpio_set_direction(PIN_NUM_LCD_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_LCD_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(PIN_NUM_LCD_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    st7701_init_sequence(io_handle);

    esp_lcd_rgb_panel_config_t rgb_cfg = {
        .data_width = 16,
        .num_fbs = 1,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .timings = {
            .pclk_hz = 10 * 1000 * 1000,
            .h_res = LCD_H_RES,
            .v_res = LCD_V_RES,
            .hsync_pulse_width = 10,
            .hsync_back_porch = 20,
            .hsync_front_porch = 10,
            .vsync_pulse_width = 10,
            .vsync_back_porch = 20,
            .vsync_front_porch = 10,
            .flags = {
                .hsync_idle_low = 0,
                .vsync_idle_low = 0,
                .de_idle_high = 1,
                .pclk_active_neg = 1,
            },
        },
		.hsync_gpio_num = PIN_NUM_HSYNC,
		.vsync_gpio_num = PIN_NUM_VSYNC,
		.de_gpio_num = PIN_NUM_DE,
//		.disp_gpio_num = PIN_NUM_DE, // aen need define
		.pclk_gpio_num = PIN_NUM_PCLK,
		.data_gpio_nums = {
			rgb_pins[0],
			rgb_pins[1],
			rgb_pins[2],
			rgb_pins[3],
			rgb_pins[4],
			rgb_pins[5],
			rgb_pins[6],
			rgb_pins[7],
			rgb_pins[8],
			rgb_pins[9],
			rgb_pins[10],
			rgb_pins[11],
			rgb_pins[12],
			rgb_pins[13],
			rgb_pins[14],
			rgb_pins[15],
			rgb_pins[16],
			rgb_pins[17]
		},
        .flags = {
            .fb_in_psram = 1,
        },
    };

    esp_lcd_panel_handle_t rgb_panel = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&rgb_cfg, &rgb_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(rgb_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(rgb_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(rgb_panel, true));

    return rgb_panel;
}
