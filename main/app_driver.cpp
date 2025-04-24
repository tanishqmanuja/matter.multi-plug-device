/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <vector>
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

std::vector<gpio_num_t> gpio = {GPIO_CHANNEL_1, GPIO_CHANNEL_2, GPIO_CHANNEL_3, GPIO_CHANNEL_4};

static void app_driver_button_toggle_cb(void *arg, void *data) {
    ESP_LOGI(TAG, "Toggle button pressed");
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val) {
    esp_err_t err = ESP_OK;
    
    if (cluster_id == OnOff::Id) {
        if (attribute_id == OnOff::Attributes::OnOff::Id) {
           int gpio_index = get_gpio_index(endpoint_id);
           if (gpio_index != -1){
                ESP_LOGI(TAG, "Toggling GPIO: %d, Val : %d", gpio.at(gpio_index), val->val.b);
                gpio_set_level(gpio.at(gpio_index), !val->val.b);
           } 
        }
    }

    return err;
}

app_driver_handle_t app_driver_plugin_unit_init(gpio_plug* plug) {
    /* Initialize plug */
    gpio_set_direction(plug->GPIO_PIN_VALUE, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(plug->GPIO_PIN_VALUE, GPIO_PULLUP_ONLY);
    return (app_driver_handle_t)plug;
}

esp_err_t app_driver_plugin_unit_set_defaults(uint16_t endpoint_id, gpio_plug* plug) {
    esp_err_t err = ESP_OK;

    gpio_num_t gpio_num= plug->GPIO_PIN_VALUE;
    if (gpio_num != GPIO_NUM_NC){
        node_t *node = node::get();
        endpoint_t *endpoint = endpoint::get(node, endpoint_id);
        cluster_t *cluster = cluster::get(endpoint, OnOff::Id);
        attribute_t *attribute = attribute::get(cluster, OnOff::Attributes::OnOff::Id);

        attribute::set_deferred_persistence(attribute);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);

        err |= gpio_set_level(gpio_num, !val.val.b);
    } 

    return err;
}

app_driver_handle_t app_driver_plugin_unit_power_supply_init() {
    gpio_set_direction(GPIO_CHANNEL_POWER, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_CHANNEL_POWER, true);
    return nullptr;
}

app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_handle_t handle = NULL;
    const button_config_t btn_cfg = {0};
    const button_gpio_config_t btn_gpio_cfg = button_driver_get_config();

    if (iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create button device");
        return NULL;
    }

    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, NULL, app_driver_button_toggle_cb, NULL);
    return (app_driver_handle_t)handle;
}
