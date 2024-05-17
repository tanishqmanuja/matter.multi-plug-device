/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <cstdint>
#include <vector>
#include "esp_err.h"
#include "esp_matter_core.h"
#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <device.h>
#include <esp_matter.h>

#include <app_priv.h>

#include "driver/gpio.h"
#include "soc/gpio_num.h"

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";

std::vector<gpio_num_t> gpio = {GPIO_CHANNEL_1, GPIO_CHANNEL_2, GPIO_CHANNEL_3, GPIO_CHANNEL_4};

// static esp_err_t app_driver_set_plugin_unit_on_off(app_driver_handle_t driver_handle, esp_matter_attr_val_t *val){
//     gpio_plug* plug = (gpio_plug*)driver_handle;
//     return gpio_set_level((gpio_num_t) plug->GPIO_PIN_VALUE, val->val.b);
// }

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    
    if (cluster_id == OnOff::Id) {
        if (attribute_id == OnOff::Attributes::OnOff::Id) {
           int gpio_index = get_gpio_index(endpoint_id);
           if (gpio_index != -1){
                gpio_set_level(gpio.at(gpio_index), !val->val.b);
           } 
        }
    }

    return err;
}

app_driver_handle_t app_driver_plugin_unit_init(gpio_plug* plug)
{
    /* Initialize plug */
    gpio_set_direction(plug->GPIO_PIN_VALUE, GPIO_MODE_OUTPUT);
    gpio_set_level(plug->GPIO_PIN_VALUE, DEFAULT_POWER);
    return (app_driver_handle_t)plug;
}

esp_err_t app_driver_plugin_unit_set_defaults(uint16_t endpoint_id, gpio_plug* plug) {
    esp_err_t err = ESP_OK;
    // int gpio_index = get_gpio_index(endpoint_id);
    gpio_num_t gpio_num= plug->GPIO_PIN_VALUE;

    // ESP_LOGI(TAG, "Restore Endpoint id: %d, val : %d", endpoint_id, gpio_num);

    if (gpio_num != GPIO_NUM_NC){
        node_t *node = node::get();
        endpoint_t *endpoint = endpoint::get(node, endpoint_id);
        cluster_t *cluster = cluster::get(endpoint, OnOff::Id);
        attribute_t *attribute = attribute::get(cluster, OnOff::Attributes::OnOff::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);

        err |= gpio_set_level(gpio_num, !val.val.b);

        // ESP_LOGI(TAG, "Restored Endpoint id: %d, val : %d", endpoint_id, val.val.b);
    } 
    return err;
}

app_driver_handle_t app_driver_plugin_unit_power_init() {
    gpio_set_direction(GPIO_CHANNEL_POWER, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_CHANNEL_POWER, true);
    return nullptr;
}

app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_config_t config = button_driver_get_config();
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_toggle_cb, NULL);
    return (app_driver_handle_t)handle;
}
