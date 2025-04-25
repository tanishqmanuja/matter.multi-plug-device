/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_matter_attribute_utils.h"
#include "esp_matter_core.h"
#include <cstdint>
#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <device.h>
#include <esp_matter.h>

#include <app_priv.h>

#include "app-common/zap-generated/ids/Attributes.h"
#include "driver/gpio.h"

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");

    bool are_all_plugs_on = true;
    for (int i = 0; i < configure_plugs; i++) {
        uint16_t endpoint_id = plugin_unit_list[i].endpoint_id;
        attribute_t *attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);

        if (!val.val.b) {
            are_all_plugs_on = false;
            break;
        }
    }

    for (int i = 0; i < configure_plugs; i++) {
        uint16_t endpoint_id = plugin_unit_list[i].endpoint_id;
        esp_matter_attr_val_t val = esp_matter_bool(!are_all_plugs_on);
        attribute::update(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id, &val);
    }
}

static esp_err_t app_driver_update_gpio_value(gpio_num_t pin, bool value)
{
    esp_err_t err = ESP_OK;

#if CONFIG_ACTIVE_HIGH_PLUGS
    bool v = !value;
#else
    bool v = value;
#endif

    err = gpio_set_level(pin, v);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO level");
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "GPIO pin : %d set to %d", pin, v);
    }
    return err;
}

esp_err_t app_driver_plugin_unit_init(const gpio_plug *plug)
{
    esp_err_t err = ESP_OK;

    /* Reset GPIO pin */
    err = gpio_reset_pin(plug->GPIO_PIN_VALUE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Unable to reset GPIO pin");
        return ESP_FAIL;
    }

    /* Initialize plug */
    err = gpio_set_direction(plug->GPIO_PIN_VALUE, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Unable to set GPIO OUTPUT mode");
        return ESP_FAIL;
    }

#if CONFIG_ACTIVE_HIGH_PLUGS
    /* Set pullup only mode */
    err = gpio_set_pull_mode(plug->GPIO_PIN_VALUE, GPIO_PULLUP_ONLY);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Unable to set GPIO pull mode");
        return ESP_FAIL;
    }
#endif

    err |= app_driver_update_gpio_value(plug->GPIO_PIN_VALUE, 0);

    return err;
}

// Return GPIO pin from plug-endpoint mapping list
gpio_num_t get_gpio(uint16_t endpoint_id)
{
    gpio_num_t gpio_pin = GPIO_NUM_NC;
    for (int i = 0; i < configure_plugs; i++) {
        if (plugin_unit_list[i].endpoint_id == endpoint_id) {
            gpio_pin = plugin_unit_list[i].plug;
        }
    }
    return gpio_pin;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;

    if (cluster_id == OnOff::Id) {
        if (attribute_id == OnOff::Attributes::OnOff::Id) {
            gpio_num_t gpio_pin = get_gpio(endpoint_id);
            if (gpio_pin != GPIO_NUM_NC) {
                err = app_driver_update_gpio_value(gpio_pin, val->val.b);
            } else {
                ESP_LOGE(TAG, "GPIO pin mapping for endpoint_id: %d not found", endpoint_id);
                return ESP_FAIL;
            }
        }
    }
    return err;
}

esp_err_t app_driver_plugin_unit_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;

    gpio_num_t gpio_num = get_gpio(endpoint_id);

    if (gpio_num == GPIO_NUM_NC) {
        ESP_LOGE(TAG, "GPIO pin mapping for endpoint_id: %d not found", endpoint_id);
        return ESP_FAIL;
    }

    attribute_t *attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
    attribute::set_deferred_persistence(attribute);

    attribute_t *startup_attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::StartUpOnOff::Id);
    esp_matter_attr_val_t startup_val = esp_matter_invalid(NULL);
    attribute::get_val(startup_attribute, &startup_val);

    if (startup_val.val.u8 == UINT8_MAX) {
        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        err |= app_driver_update_gpio_value(gpio_num, val.val.b);
    } else {
        err |= app_driver_update_gpio_value(gpio_num, startup_val.val.b);
    }

    return err;
}

esp_err_t app_driver_plugin_unit_power_supply_init()
{
    esp_err_t err = ESP_OK;

    err = gpio_set_direction((gpio_num_t)CONFIG_GPIO_POWER_OUTLET, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Unable to set GPIO OUTPUT mode");
        return ESP_FAIL;
    }

    err = gpio_set_level((gpio_num_t)CONFIG_GPIO_POWER_OUTLET, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Unable to set GPIO level");
    }

    return err;
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
