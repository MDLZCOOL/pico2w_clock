#include "pico/stdlib.h"
#include <stdio.h>
#include <pico/cyw43_arch.h>
#include "lvgl.h"
#include "lv_port_disp.h"
#include "gui_guider.h"
#include "wifi.h"
#include "lib/ntp/ntp.h"
#include <time.h>
#include "bsp_st7306.h"

#define LV_TICK_PERIOD_MS 5

static struct repeating_timer lv_timer;
static struct repeating_timer rtc_timer;
static struct repeating_timer ntp_timer;
lv_ui guider_ui;
int g_link_state = CYW43_LINK_FAIL;
struct tm g_time;

bool lv_timer_callback(struct repeating_timer *t) {
    lv_tick_inc(LV_TICK_PERIOD_MS);
    return true;
}

bool rtc_timer_callback(struct repeating_timer *t) {
    printf("1s has passed...\n");
    if (g_time.tm_year > 0) {
        int old_minute = g_time.tm_min;
        g_time.tm_sec++;
        mktime(&g_time);
        if (g_time.tm_min != old_minute) {
            printf("Minute changed, updating display to %02d:%02d\n", g_time.tm_hour, g_time.tm_min);
            lv_label_set_text_fmt(guider_ui.screen_label_1, "%02d", g_time.tm_hour);
            lv_label_set_text_fmt(guider_ui.screen_label_3, "%02d", g_time.tm_min);
        }
    }
    return true;
}

bool ntp_sync_timer_callback(struct repeating_timer *t) {
    printf("12h has passed...\n");
#if 0
    if (g_link_state == CYW43_LINK_UP) {
        ntp_status_t status = ntp_get_time(&g_time, 30000); // 30s timeout
        if (status == NTP_SUCCESS) {
            printf("NTP time synced successfully!\n");
            printf("UTC Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                   g_time.tm_year + 1900,
                   g_time.tm_mon + 1,
                   g_time.tm_mday,
                   g_time.tm_hour,
                   g_time.tm_min,
                   g_time.tm_sec);
        }
        else {
            printf("NTP status: %d\n", status);
        }
    }
#endif
    return true;
}
void bsp_st7306_draw_filled_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    // Basic boundary checks
    if (x0 >= BSP_ST7306_SCREEN_WIDTH || y0 >= BSP_ST7306_SCREEN_HEIGHT) {
        return;
    }
    if (x1 >= BSP_ST7306_SCREEN_WIDTH) {
        x1 = BSP_ST7306_SCREEN_WIDTH - 1;
    }
    if (y1 >= BSP_ST7306_SCREEN_HEIGHT) {
        y1 = BSP_ST7306_SCREEN_HEIGHT - 1;
    }

    for (uint16_t y = y0; y <= y1; y++) {
        for (uint16_t x = x0; x <= x1; x++) {
            bsp_st7306_drawpoint(x, y, color);
        }
    }
}
int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("System begin...\n");
    add_repeating_timer_ms(-LV_TICK_PERIOD_MS, lv_timer_callback, NULL, &lv_timer);
    add_repeating_timer_ms(1000, rtc_timer_callback, NULL, &rtc_timer);
    add_repeating_timer_ms(-1000 * 60, ntp_sync_timer_callback, NULL, &ntp_timer);

    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000);

    lv_init();
    lv_port_disp_init();
    setup_ui(&guider_ui);

    g_link_state = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    if (g_link_state == CYW43_LINK_UP) {
        // try to get time when power on and wi-fi connected
        ntp_status_t status = ntp_get_time(&g_time, 30000); // 30s timeout
        if (status == NTP_SUCCESS) {
            printf("NTP time synced successfully!\n");
            printf("UTC Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                   g_time.tm_year + 1900,
                   g_time.tm_mon + 1,
                   g_time.tm_mday,
                   g_time.tm_hour,
                   g_time.tm_min,
                   g_time.tm_sec);
            g_time.tm_hour += 8; // UTC+8 beijing time
            mktime(&g_time);
            lv_label_set_text_fmt(guider_ui.screen_label_1, "%02d", g_time.tm_hour);
            lv_label_set_text_fmt(guider_ui.screen_label_3, "%02d", g_time.tm_min);
            lv_label_set_text_fmt(guider_ui.screen_label_4, "%d年%d月%d日 周%d", g_time.tm_year + 1900, g_time.tm_mon + 1, g_time.tm_mday, g_time.tm_wday);
        }
        lv_label_set_text_fmt(guider_ui.screen_label_8, "WiFi: %s", WIFI_SSID);
    }

    while (1) {
//        printf("System running...\n");
        lv_task_handler();
    }
    return 0;
}