/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include <hal/gpio_types.h>
#include "soc/gpio_num.h"

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif

#define DEVICE_NAME "Multi Plug Device"

/* Plugs state on when initializing */
#define DEFAULT_ON_OFF false

struct gpio_plug {
    gpio_num_t GPIO_PIN_VALUE;
};

struct plugin_endpoint {
    uint16_t endpoint_id;
    gpio_num_t plug;
};

gpio_num_t get_gpio(uint16_t endpoint_id);

extern plugin_endpoint plugin_unit_list[CONFIG_NUM_PLUGS];
extern uint16_t configure_plugs;

typedef void *app_driver_handle_t;

/** Initialize the plug driver
 *
 * This initializes the plug driver associated with the selected board.
 *
 * @return Handle on success.
 * @return NULL in case of failure.
 */
esp_err_t app_driver_plugin_unit_init(const gpio_plug *plug);

/** Set default values for the plug driver
 *
 * @param[in] endpoint_id Endpoint ID of the attribute.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_plugin_unit_set_defaults(uint16_t endpoint_id);

/** Initialize the power supply driver
 *
 * This initializes the power supply driver associated with the relay module.
 *
 * @return Handle on success.
 * @return NULL in case of failure.
 */
esp_err_t app_driver_plugin_unit_power_supply_init();

/** Initialize the button driver
 *
 * This initializes the button driver associated with the selected board.
 *
 * @return Handle on success.
 * @return NULL in case of failure.
 */
app_driver_handle_t app_driver_button_init();

/** Driver Update
 *
 * This API should be called to update the driver for the attribute being updated.
 * This is usually called from the common `app_attribute_update_cb()`.
 *
 * @param[in] endpoint_id Endpoint ID of the attribute.
 * @param[in] cluster_id Cluster ID of the attribute.
 * @param[in] attribute_id Attribute ID of the attribute.
 * @param[in] val Pointer to `esp_matter_attr_val_t`. Use appropriate elements as per the value type.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG() \
    {                                         \
        .radio_mode = RADIO_MODE_NATIVE,      \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()               \
    {                                                      \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE, \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG() \
    {                                        \
        .storage_partition_name = "nvs",     \
        .netif_queue_size = 10,              \
        .task_queue_size = 10,               \
    }
#endif
