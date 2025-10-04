#include <inttypes.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define SENSOR_CHANNEL ADC1_CHANNEL_6   // GPIO34 (LDR input)
#define LED_PIN        4              // GPIO4(LED output)
#define DEFAULT_VREF   1100             // mV
#define ADC_WIDTH      ADC_WIDTH_BIT_12 // 12-bit (0-4095)
#define ADC_ATTEN      ADC_ATTEN_DB_11  // 0 ~ 3.3V

static const char *TAG = "LDR_LED";
static esp_adc_cal_characteristics_t *adc_chars;

void app_main(void)
{
    // --- ตั้งค่า LED pin ---
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // --- ตั้งค่า ADC ---
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(SENSOR_CHANNEL, ADC_ATTEN);

    // --- คาลิเบรต ADC ---
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
        ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, adc_chars
    );

    ESP_LOGI(TAG, "Start LDR + LED test");

    while (1) {
        // อ่านค่า ADC raw
        uint32_t raw = adc1_get_raw(SENSOR_CHANNEL);

        // แปลงเป็นแรงดัน (โวลต์)
        uint32_t mv = esp_adc_cal_raw_to_voltage(raw, adc_chars);
        float volt = mv / 1000.0f;

        // แสดงผล
        ESP_LOGI(TAG, "ADC Raw: %" PRIu32 ", Voltage: %.2f V", raw, volt);

        // ตรวจเงื่อนไข (ค่าต่ำกว่า 1000 -> มืด -> LED ติด)
        if (raw < 1000) {
            gpio_set_level(LED_PIN, 1);  // เปิด LED
            ESP_LOGI(TAG, "LED ON (Dark detected!)");
        } else {
            gpio_set_level(LED_PIN, 0);  // ปิด LED
            ESP_LOGI(TAG, "LED OFF (Bright enough)");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // ดีเลย์ 1 วินาที
    }
}
