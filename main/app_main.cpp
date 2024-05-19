/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <common_macros.h>
#include <app_priv.h>
#include <app_reset.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

static const char *TAG = "app_main";
static uint16_t configured_plugs = 0;
static plugin_unit_endpoint plugin_unit_list[CONFIG_MAX_CONFIGURABLE_PLUGS];

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg) {
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
        {
            ESP_LOGI(TAG, "Fabric removed successfully");
            if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
            {
                chip::CommissioningWindowManager & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
                constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
                if (!commissionMgr.IsCommissioningWindowOpen())
                {
                    /* After removing last fabric, this example does not remove the Wi-Fi credentials
                     * and still has IP connectivity so, only advertising on DNS-SD.
                     */
                    CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                    chip::CommissioningWindowAdvertisement::kDnssdOnly);
                    if (err != CHIP_NO_ERROR)
                    {
                        ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                    }
                }
            }
        break;
        }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG, "Fabric is committed");
        break;
    default:
        break;
    }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data) {
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

// This callback is called for every attribute update. The callback implementation shall
// handle the desired attributes and return an appropriate error code. If the attribute
// is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data) {
    esp_err_t err = ESP_OK;

    if (type == PRE_UPDATE) {
        /* Driver update */
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
    }

    return err;
}

static esp_err_t create_plug(struct gpio_plug* plug, node_t* node) {
    esp_err_t err = ESP_OK;

    /* Initialize driver */
    app_driver_handle_t plugin_unit_handle = app_driver_plugin_unit_init(plug);

    /* Create a new endpoint. */
    on_off_plugin_unit::config_t plugin_unit_config;
    plugin_unit_config.on_off.on_off = DEFAULT_POWER;
    plugin_unit_config.on_off.lighting.start_up_on_off = nullptr; // Uses previously saved value from flash
    endpoint_t *endpoint = on_off_plugin_unit::create(node, &plugin_unit_config, ENDPOINT_FLAG_NONE, plugin_unit_handle);

    /* These node and endpoint handles can be used to create/add other endpoints and clusters. */
    if (!node || !endpoint)
    {
        ESP_LOGE(TAG, "Matter node creation failed");
        err = ESP_FAIL;
        return err;
    }

    for (int i = 0; i < configured_plugs; i++) {
        if (plugin_unit_list[i].plug == plug) {
            ESP_LOGI(TAG, "Plug already configured: %d", endpoint::get_id(endpoint));
            return ESP_OK;
        }
    }

    /* Check for maximum physical buttons that can be configured. */
    if (configured_plugs <CONFIG_MAX_CONFIGURABLE_PLUGS) {
        plugin_unit_list[configured_plugs].plug = plug;
        plugin_unit_list[configured_plugs].endpoint_id = endpoint::get_id(endpoint);
        app_driver_plugin_unit_set_defaults(endpoint::get_id(endpoint), plug);
        configured_plugs++;
    } else {
        ESP_LOGI(TAG, "Cannot configure more plugs");
        err = ESP_FAIL;
        return err;
    }

    static uint16_t plug_endpoint_id = 0;
    plug_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Plug created with endpoint_id %d", plug_endpoint_id);

    cluster::fixed_label::config_t fl_config;
    cluster::fixed_label::create(endpoint, &fl_config, CLUSTER_FLAG_SERVER);

    return err;
}

int get_gpio_index(uint16_t endpoint_id) {
    for(int i = 0; i < configured_plugs; i++) {
        if (plugin_unit_list[i].endpoint_id == endpoint_id) {
            return i;
        }
    }
    return -1;
}

extern "C" void app_main() {
    esp_err_t err = ESP_OK;

    /* Initialize the ESP NVS layer */
    nvs_flash_init();

    /* Initialize driver */
    app_driver_handle_t button_handle = app_driver_button_init();
    app_reset_button_register(button_handle);

    /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
    node::config_t node_config;
    snprintf(node_config.root_node.basic_information.node_label, sizeof(node_config.root_node.basic_information.node_label), DEVICE_NAME);
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));

    /* Create and initialize plugs */
    struct gpio_plug plug1 = { .GPIO_PIN_VALUE = GPIO_CHANNEL_1 };
    create_plug(&plug1, node);

    struct gpio_plug plug2 = { .GPIO_PIN_VALUE = GPIO_CHANNEL_2 };
    create_plug(&plug2, node);

    struct gpio_plug plug3 = { .GPIO_PIN_VALUE = GPIO_CHANNEL_3 };
    create_plug(&plug3, node);

    struct gpio_plug plug4 = { .GPIO_PIN_VALUE = GPIO_CHANNEL_4 };
    create_plug(&plug4, node);

    /* Initialize plug power supply */
    app_driver_plugin_unit_power_supply_init();

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
#endif

    /* Matter start */
    err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::init();
#endif
}
