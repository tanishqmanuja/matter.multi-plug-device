/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <device.h>
#include <esp_matter.h>

#include <app_priv.h>

#include "driver/gpio.h"

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;

    if(endpoint_id == 0) {
        return ESP_OK;
    }

    bool state = val->val.b;
    gpio_plug* plug = get_gpio_plug(endpoint_id);

    if(plug != nullptr) {
        err = gpio_set_level((gpio_num_t) plug->GPIO_PIN_VALUE,state);
        if(err == ESP_OK) {
            ESP_LOGE(TAG,"toggling GPIO %d", (gpio_num_t)plug->GPIO_PIN_VALUE);
            ESP_LOGE(TAG, "SET Endpoint %d to %d", endpoint_id, state);
        } else {
            ESP_LOGE(TAG,"Error toggling GPIO %d, error %d", (gpio_num_t)plug->GPIO_PIN_VALUE, err);
        }
    } else {
        ESP_LOGE(TAG,"Error finding GPIO for endpoint %d", endpoint_id);
        return ESP_FAIL;
    }

    return err;
}

app_driver_handle_t app_driver_plug_init(gpio_plug * plug)
{
    esp_err_t err = gpio_set_direction(plug->GPIO_PIN_VALUE, GPIO_MODE_OUTPUT);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Error initializing GPIO %d", plug->GPIO_PIN_VALUE);
        return nullptr; // Enable restore prev value from flash
    }
    return (app_driver_handle_t)plug;
}

app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_config_t config = button_driver_get_config();
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_toggle_cb, NULL);
    return (app_driver_handle_t)handle;
}
