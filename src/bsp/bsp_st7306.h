#ifndef PICO2W_CLOCK_BSP_ST7306_H
#define PICO2W_CLOCK_BSP_ST7306_H

#define BSP_ST7306_USE_DMA 1

#define BSP_ST7306_SCREEN_WIDTH    300
#define BSP_ST7306_SCREEN_HEIGHT   400

#define BSP_ST7306_COLOR_WHITE     0
#define BSP_ST7306_COLOR_BLACK     1

#define BSP_ST7306_CS_PIN PICO_DEFAULT_SPI_CSN_PIN
#define BSP_ST7306_DC_PIN 21
#define BSP_ST7306_RESET_PIN 20
#define BSP_ST7306_SPI_MOSI_PIN PICO_DEFAULT_SPI_TX_PIN
#define BSP_ST7306_SPI_MISO_PIN PICO_DEFAULT_SPI_RX_PIN
#define BSP_ST7306_SPI_SCK_PIN PICO_DEFAULT_SPI_SCK_PIN

void bsp_st7306_init();

void bsp_st7306_drawpoint(uint16_t x, uint16_t y, uint16_t color);

void bsp_st7306_drawline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

void bsp_st7306_update();

void bsp_st7306_fill(uint16_t color);

#if BSP_ST7306_USE_DMA
void bsp_st7306_wait_for_dma_finish();
#endif

#endif //PICO2W_CLOCK_BSP_ST7306_H
