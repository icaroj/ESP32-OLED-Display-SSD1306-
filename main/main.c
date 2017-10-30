#include "freertos/FreeRTOS.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "nvs_flash.h"

#include "driver/gpio.h"
#include "driver/i2c.h"

#include "config.h"
#include "ssd1306.h"

char DISPLAY_TITLE[] = "DISPLAY";
esp_err_t event_handler(void *ctx, system_event_t *event) {
        return ESP_OK;
}

void app_main(void) {
        nvs_flash_init();
        tcpip_adapter_init();
        ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
        ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
        ESP_ERROR_CHECK( esp_wifi_start() );
        ESP_ERROR_CHECK( esp_wifi_connect() );

        // Oled display
        if(oled_ssd1306_init() == ESP_OK){
                ESP_LOGI(DISPLAY_TITLE, "On!");
        }
        else {
                ESP_LOGI(DISPLAY_TITLE, "Off!");
        }

                xTaskCreate(&oled_ssd1306_clear, "oled_ssd1306_clear",  2048, NULL, 6, NULL);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                xTaskCreate(&oled_ssd1306_screensaver, "oled_ssd1306_screensaver",  2048, NULL, 6, NULL);
                // xTaskCreate(&oled_ssd1306_print, "oled_ssd1306_print",  2048, (void *)"Atirei o pau no\ngato, mas o gato\nnao morreu...\nDona Chica admi-\nrou-se do berro\nque o gato deu\nMIAAAAU!", 6, NULL);
        while (true) {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}
