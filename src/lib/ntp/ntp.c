#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "ntp.h"

typedef struct NTP_T_ {
    ip_addr_t ntp_server_address;
    struct udp_pcb *ntp_pcb;
    volatile bool response_received;
    volatile ntp_status_t result_status;
    time_t epoch_time;
} NTP_T;

#define NTP_SERVER "ntp.aliyun.com"
#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970

static void ntp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    NTP_T *state = (NTP_T *) arg;
    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    if (ip_addr_cmp(addr, &state->ntp_server_address) && port == NTP_PORT && p->tot_len == NTP_MSG_LEN &&
        mode == 0x4 && stratum != 0) {

        uint8_t seconds_buf[4] = {0};
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 =
                seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
        uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;

        state->epoch_time = seconds_since_1970;
        state->result_status = NTP_SUCCESS;
    } else {
        state->result_status = NTP_FAILED_INVALID;
    }

    state->response_received = true;
    pbuf_free(p);
}

static void ntp_dns_found_callback(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    NTP_T *state = (NTP_T *) arg;
    if (ipaddr) {
        state->ntp_server_address = *ipaddr;
        state->result_status = NTP_SUCCESS;

        cyw43_arch_lwip_begin();
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
        uint8_t *req = (uint8_t *) p->payload;
        memset(req, 0, NTP_MSG_LEN);
        req[0] = 0x1b; // li=0, vn=3, mode=3
        udp_sendto(state->ntp_pcb, p, &state->ntp_server_address, NTP_PORT);
        pbuf_free(p);
        cyw43_arch_lwip_end();

    } else {
        state->result_status = NTP_FAILED_DNS;
        state->response_received = true;
    }
}

ntp_status_t ntp_get_time(struct tm *result_tm, uint32_t timeout_ms) {
    NTP_T *state = (NTP_T *) calloc(1, sizeof(NTP_T));
    if (!state) {
        return NTP_FAILED_INIT;
    }

    state->response_received = false;
    state->result_status = NTP_FAILED_REQUEST;

    cyw43_arch_lwip_begin();
    state->ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!state->ntp_pcb) {
        cyw43_arch_lwip_end();
        free(state);
        return NTP_FAILED_INIT;
    }
    udp_recv(state->ntp_pcb, ntp_recv_callback, state);

    int err = dns_gethostbyname(NTP_SERVER, &state->ntp_server_address, ntp_dns_found_callback, state);
    cyw43_arch_lwip_end();

    if (err == ERR_OK) {
    } else if (err != ERR_INPROGRESS) {
        udp_remove(state->ntp_pcb);
        free(state);
        return NTP_FAILED_DNS;
    }

    absolute_time_t timeout_time = make_timeout_time_ms(timeout_ms);
    while (absolute_time_diff_us(get_absolute_time(), timeout_time) > 0) {
        if (state->response_received) {
            break;
        }
        sleep_ms(1);
    }

    ntp_status_t final_status = state->result_status;

    if (final_status == NTP_SUCCESS) {
        struct tm *utc = gmtime(&state->epoch_time);
        if (utc && result_tm) {
            memcpy(result_tm, utc, sizeof(struct tm));
        } else {
            final_status = NTP_FAILED_REQUEST;
        }
    }

    cyw43_arch_lwip_begin();
    udp_remove(state->ntp_pcb);
    cyw43_arch_lwip_end();

    free(state);

    return final_status;
}
