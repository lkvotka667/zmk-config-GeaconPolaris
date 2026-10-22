/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>
#include <stdbool.h>

struct led4_hsl { uint16_t h; uint8_t s, l; };

/* 0=rainbow, 1=connection chase, 2=batteries until idle. */
static inline int led4_phase(uint64_t ms) { return ms < 2000 ? 0 : ms < 4000 ? 1 : 2; }
static inline struct led4_hsl led4_battery(int level, uint64_t ms) {
    if (level < 0 || level > 100) {
        unsigned t = ms % 2000;
        return (struct led4_hsl){0, 0, 8 + (t < 1000 ? t : 2000 - t) * 12 / 1000};
    }
    if (level >= 80) return (struct led4_hsl){200, 90, 55};
    if (level >= 30) return (struct led4_hsl){120, 100, 35};
    return (struct led4_hsl){0, 100, 35};
}

static inline struct led4_hsl led4_pixel(uint64_t ms, unsigned pixel,
                                        bool connected, int left, int right) {
    if (led4_phase(ms) == 0) {
        unsigned fade = ms < 500 ? ms : 500;
        return (struct led4_hsl){(ms * 180 / 1000 + pixel * 60) % 360, 90,
                                30 * fade / 500};
    }
    if (led4_phase(ms) == 1) {
        /* Two gentle sweeps, always 1 -> 4, never back and forth. */
        unsigned t = (ms - 2000) % 1000;
        unsigned head = t / 250;
        unsigned within = t % 250;
        unsigned light = pixel == head ? 10 + 30 * within / 250 :
                         pixel + 1 == head ? 40 - 30 * within / 250 : 3;
        return (struct led4_hsl){connected ? 200 : 0, 100, light};
    }
    return led4_battery(pixel < 2 ? left : right, ms);
}
