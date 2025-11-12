#ifndef PICO2W_CLOCK_NTP_H
#define PICO2W_CLOCK_NTP_H

#include <time.h>

typedef enum {
    NTP_SUCCESS = 0,
    NTP_FAILED_DNS = -1,        // DNS 解析失败
    NTP_FAILED_REQUEST = -2,    // NTP 请求失败 (超时)
    NTP_FAILED_INVALID = -3,    // 收到无效的 NTP 响应
    NTP_FAILED_INIT = -4        // 初始化失败 (内存不足等)
} ntp_status_t;

ntp_status_t ntp_get_time(struct tm *result_tm, uint32_t timeout_ms);

#endif //PICO2W_CLOCK_NTP_H