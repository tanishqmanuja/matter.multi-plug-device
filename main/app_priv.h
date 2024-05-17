/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <cstdint>
#include <esp_err.h>
#include <esp_matter.h>
#include <hal/gpio_types.h>
#include "soc/gpio_num.h"

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif

#define DEFAULT_POWER false

#define GPIO_CHANNEL_1 (gpio_num_t)GPIO_NUM_2
#define GPIO_CHANNEL_2 (gpio_num_t)GPIO_NUM_12
#define GPIO_CHANNEL_3 (gpio_num_t)GPIO_NUM_13
#define GPIO_CHANNEL_4 (gpio_num_t)GPIO_NUM_14

struct gpio_plug
{
      gpio_num_t GPIO_PIN_VALUE;
};

struct plugin_unit_endpoint
{
    uint16_t endpoint_id;
    gpio_plug* plug;
};

extern int get_gpio_index(uint16_t endpoint_id);

typedef void *app_driver_handle_t;

/** Initialize the plug driver
 *
 * This initializes the plug driver associated with the selected board.
 *
 * @return Handle on success.
 * @return NULL in case of failure.
 */
app_driver_handle_t app_driver_plugin_unit_init(gpio_plug* plug);

/** Set default values for the plug driver
 *
 * @param[in] endpoint_id Endpoint ID of the attribute.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_plugin_unit_set_defaults(uint16_t endpoint_id, gpio_plug* plug);

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
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
    {                                                                                   \
        .radio_mode = RADIO_MODE_NATIVE,                                                \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
    {                                                                                   \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
    {                                                                                   \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
    }
#endif
