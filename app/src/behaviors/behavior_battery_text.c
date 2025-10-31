/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_battery_text

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/battery.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
#include <zmk/split/central.h>
#endif

#include <dt-bindings/zmk/keys.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// Helper function to get the key code for a digit (0-9)
static uint32_t digit_to_keycode(uint8_t digit) {
    switch (digit) {
    case 0:
        return N0;
    case 1:
        return N1;
    case 2:
        return N2;
    case 3:
        return N3;
    case 4:
        return N4;
    case 5:
        return N5;
    case 6:
        return N6;
    case 7:
        return N7;
    case 8:
        return N8;
    case 9:
        return N9;
    default:
        return N0;
    }
}

// Helper function to queue a single character
static int queue_char(const struct zmk_behavior_binding_event *event, uint32_t keycode,
                      uint32_t tap_ms, uint32_t wait_ms) {
    struct zmk_behavior_binding binding = {
        .behavior_dev = "kp",
        .param1 = keycode,
    };

    zmk_behavior_queue_add(event, binding, true, tap_ms);
    zmk_behavior_queue_add(event, binding, false, wait_ms);

    return 0;
}

// Queue a percentage value (0-100) as text
static int queue_percentage(const struct zmk_behavior_binding_event *event, uint8_t percentage,
                            uint32_t tap_ms, uint32_t wait_ms) {
    if (percentage >= 100) {
        queue_char(event, digit_to_keycode(1), tap_ms, wait_ms);
        queue_char(event, digit_to_keycode(0), tap_ms, wait_ms);
        queue_char(event, digit_to_keycode(0), tap_ms, wait_ms);
    } else if (percentage >= 10) {
        uint8_t tens = percentage / 10;
        uint8_t ones = percentage % 10;
        queue_char(event, digit_to_keycode(tens), tap_ms, wait_ms);
        queue_char(event, digit_to_keycode(ones), tap_ms, wait_ms);
    } else {
        queue_char(event, digit_to_keycode(percentage), tap_ms, wait_ms);
    }

    return 0;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    LOG_DBG("Battery text behavior pressed");

    const uint32_t tap_ms = 10;
    const uint32_t wait_ms = 10;

    // Get local (central) battery level
    uint8_t local_battery = zmk_battery_state_of_charge();
    LOG_DBG("Local battery level: %d%%", local_battery);

    // Get peripheral battery level
    uint8_t peripheral_battery = 0;
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
    int ret = zmk_split_central_get_peripheral_battery_level(0, &peripheral_battery);
    if (ret < 0) {
        LOG_WRN("Failed to get peripheral battery level: %d", ret);
        peripheral_battery = 0;
    } else {
        LOG_DBG("Peripheral battery level: %d%%", peripheral_battery);
    }
#else
    LOG_DBG("Split battery fetching not enabled, peripheral will show 0%%");
#endif

    // Output format: "L:XX% R:YY%"

    // "L"
    queue_char(&event, LS(L), tap_ms, wait_ms);

    // ":"
    queue_char(&event, COLON, tap_ms, wait_ms);

    // Local battery percentage
    queue_percentage(&event, local_battery, tap_ms, wait_ms);

    // "%"
    queue_char(&event, PERCENT, tap_ms, wait_ms);

    // " " (space)
    queue_char(&event, SPACE, tap_ms, wait_ms);

    // "R"
    queue_char(&event, LS(R), tap_ms, wait_ms);

    // ":"
    queue_char(&event, COLON, tap_ms, wait_ms);

    // Peripheral battery percentage
    queue_percentage(&event, peripheral_battery, tap_ms, wait_ms);

    // "%"
    queue_char(&event, PERCENT, tap_ms, wait_ms);

    LOG_DBG("Battery text output queued: L:%d%% R:%d%%", local_battery, peripheral_battery);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    // Nothing to do on release
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_battery_text_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

static int behavior_battery_text_init(const struct device *dev) { return 0; }

#define BAT_TEXT_INST(n)                                                                           \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_battery_text_init, NULL, NULL, NULL, POST_KERNEL,         \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_battery_text_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BAT_TEXT_INST)
