#include "pico/stdlib.h"
#include <stdio.h>
#include "lvgl.h"
#include "lv_port_disp.h"
#include "gui_guider.h"

#define LV_TICK_PERIOD_MS 5

static struct repeating_timer lv_timer;
lv_ui guider_ui;

bool lv_timer_callback(struct repeating_timer *t) {
    lv_tick_inc(LV_TICK_PERIOD_MS);
    printf("Tick\n");
    return true;
}

int main() {
    stdio_init_all();
    printf("System begin...\n");
    add_repeating_timer_ms(-LV_TICK_PERIOD_MS, lv_timer_callback, NULL, &lv_timer);
    lv_init();
    lv_port_disp_init();
    setup_ui(&guider_ui);

    while (1) {
        lv_task_handler();
    }
    return 0;
}