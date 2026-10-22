/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT te9no_polaris_led4_status
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/bluetooth/conn.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/split/transport/central.h>
#include <zmk_driver_animation/animation.h>
#include <zmk_driver_animation/color.h>
#include <zmk_driver_animation/drivers/animation.h>
#include "led4_palette.h"

/* Do not use the split battery getter's initial zero as a measurement. */
static atomic_t left_level = ATOMIC_INIT(-1);
static atomic_t right_level = ATOMIC_INIT(-1);
static int64_t started;
static bool running;
static bool last_connected;

static bool right_connected(void) {
    STRUCT_SECTION_FOREACH(zmk_split_transport_central, transport) {
        if (!transport->api->get_status) continue;
        struct zmk_split_transport_status s = transport->api->get_status();
        if (s.available && s.enabled &&
            s.connections == ZMK_SPLIT_TRANSPORT_CONNECTIONS_STATUS_ALL_CONNECTED) return true;
    }
    return false;
}

static int battery_event(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *local = as_zmk_battery_state_changed(eh);
    if (local) atomic_set(&left_level, local->state_of_charge);
    const struct zmk_peripheral_battery_state_changed *remote =
        as_zmk_peripheral_battery_state_changed(eh);
    if (remote && remote->source == 0) atomic_set(&right_level, remote->state_of_charge);
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(polaris_led4_battery, battery_event);
ZMK_SUBSCRIPTION(polaris_led4_battery, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(polaris_led4_battery, zmk_peripheral_battery_state_changed);

/* Invalidate even while idle, so a reconnection never displays stale charge. */
static void disconnected(struct bt_conn *conn, uint8_t reason) {
    struct bt_conn_info info;
    if (!bt_conn_get_info(conn, &info) && info.role == BT_CONN_ROLE_CENTRAL)
        atomic_set(&right_level, -1);
}
BT_CONN_CB_DEFINE(polaris_led4_conn) = { .disconnected = disconnected };

static void start(const struct device *dev, uint32_t duration) {
    started = k_uptime_get();
    last_connected = right_connected();
    running = true;
    zmk_animation_request_frames(2);
}
static void stop(const struct device *dev) { running = false; }
static bool finished(const struct device *dev) { return !running; }
static void render(const struct device *dev, struct animation_pixel *pixels, size_t count) {
    if (!running || count < 4) return;
    bool connected = right_connected();
    int64_t now = k_uptime_get();
    if (connected != last_connected && now - started >= 4000) {
        /* A changed connection is more important than a steady battery display. */
        started = now - 2000;
    }
    last_connected = connected;
    int right = connected ? atomic_get(&right_level) : -1;
    for (unsigned i = 0; i < 4; i++) {
        struct led4_hsl c = led4_pixel(now - started, i, connected,
                                       atomic_get(&left_level), right);
        struct zmk_color_hsl hsl = {.h = c.h, .s = c.s, .l = c.l};
        zmk_hsl_to_rgb(&hsl, &pixels[i].value);
    }
    zmk_animation_request_frames(2);
}
static const struct animation_api api = {
    .on_start = start, .on_stop = stop, .render_frame = render, .is_finished = finished,
};
DEVICE_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                      CONFIG_APPLICATION_INIT_PRIORITY, &api);
