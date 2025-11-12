#include <hardware/spi.h>
#include <string.h>
#include <stdlib.h>
#include "bsp_st7306.h"
#include "pico/stdlib.h"

#if BSP_ST7306_USE_DMA

#include "hardware/dma.h"

#endif

uint8_t bsp_st7306_screen_buffer[BSP_ST7306_BUFFER_SIZE];

#if BSP_ST7306_USE_DMA
static int st7306_dma_channel;
static dma_channel_config st7306_dma_cfg;
#endif

static inline void cs_select() {
    gpio_put(BSP_ST7306_CS_PIN, 0);  // Active low
}

static inline void cs_deselect() {
    gpio_put(BSP_ST7306_CS_PIN, 1);
}

static inline void dc_command() {
    gpio_put(BSP_ST7306_DC_PIN, 0);  // Low for command
}

static inline void dc_data() {
    gpio_put(BSP_ST7306_DC_PIN, 1);  // High for data
}

static inline void reset_low() {
    gpio_put(BSP_ST7306_RESET_PIN, 0);
}

static inline void reset_high() {
    gpio_put(BSP_ST7306_RESET_PIN, 1);
}

static void write_command(uint8_t cmd) {
    dc_command();
    cs_select();
    spi_write_blocking(spi_default, &cmd, 1);
    cs_deselect();
}

static void write_data(uint8_t data) {
    cs_select();
    dc_data();
    spi_write_blocking(spi_default, &data, 1);
    cs_deselect();
}

void bsp_st7306_init() {
    spi_init(spi_default, 50000000);
    spi_set_format(spi_default, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(BSP_ST7306_SPI_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(BSP_ST7306_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(BSP_ST7306_SPI_MOSI_PIN, GPIO_FUNC_SPI);
#if BSP_ST7306_USE_DMA
    st7306_dma_channel = dma_claim_unused_channel(true);
    st7306_dma_cfg = dma_channel_get_default_config(st7306_dma_channel);
    channel_config_set_transfer_data_size(&st7306_dma_cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&st7306_dma_cfg, true);
    channel_config_set_write_increment(&st7306_dma_cfg, false);
    channel_config_set_dreq(&st7306_dma_cfg, spi_get_dreq(spi_default, true));  // true for tx
#endif
    gpio_init(BSP_ST7306_CS_PIN);
    gpio_set_dir(BSP_ST7306_CS_PIN, GPIO_OUT);
    gpio_init(BSP_ST7306_DC_PIN);
    gpio_set_dir(BSP_ST7306_DC_PIN, GPIO_OUT);
    gpio_init(BSP_ST7306_RESET_PIN);
    gpio_set_dir(BSP_ST7306_RESET_PIN, GPIO_OUT);

    reset_high();
    sleep_ms(10);
    reset_low();
    sleep_ms(10);
    reset_high();
    sleep_ms(10);

    // 7306
    write_command(0xD6); //NVM Load Control 
    write_data(0X17);
    write_data(0X02);

    write_command(0xD1); //Booster Enable 
    write_data(0X01);

    write_command(0xC0); //Gate Voltage Setting 
    write_data(0X12); //VGH 00:8V  04:10V  08:12V   0E:15V   12:17V
    write_data(0X0A); //VGL 00:-5V   04:-7V   0A:-10V

    write_command(0XC1);   //VSHP Setting
    write_data(0X37);  //VSHP1=4.8V
    write_data(0X37);  //VSHP2=4.8V
    write_data(0X37);  //VSHP3=4.8V
    write_data(0X37);  //VSHP4=4.8V

    write_command(0XC2);   //VSLP Setting
    write_data(0X19);  //VSLP1=0.5V
    write_data(0X19);  //VSLP2=0.5V
    write_data(0X19);  //VSLP3=0.5V
    write_data(0X19);  //VSLP4=0.5V

    write_command(0XC4);   //VSHN Setting
    write_data(0X41);  //VSHN1=-3.8V
    write_data(0X41);  //VSHN2=-3.8V
    write_data(0X41);  //VSHN3=-3.8V
    write_data(0X41);  //VSHN4=-3.8V

    write_command(0XC5);   //VSLN Setting
    write_data(0X19);  //VSLN1=0.5V
    write_data(0X19);  //VSLN2=0.5V
    write_data(0X19);  //VSLN3=0.5V
    write_data(0X19);  //VSLN4=0.5V



    // write_command(0XD8);   //OSC Setting
    // write_data(0XA6);  
    // write_data(0XE9);
    // /*-- HPM=32hz ; LPM=> 0x15=8Hz 0x14=4Hz 0x13=2Hz 0x12=1Hz 0x11=0.5Hz 0x10=0.25Hz---*/
    // write_command(0xB2); //Frame Rate Control 
    // write_data(0X15); //HPM=32hz ; LPM=8hz 



    write_command(0XD8);   //OSC Setting
    write_data(0XA6);
    write_data(0XE9);
    /*-- HPM=32hz ; LPM=> 0x15=8Hz 0x14=4Hz 0x13=2Hz 0x12=1Hz 0x11=0.5Hz 0x10=0.25Hz---*/
    write_command(0xB2); //Frame Rate Control 
    write_data(0X02); //HPM=16hz ; LPM=1hz 



    write_command(0xB3); //Update Period Gate EQ Control in HPM 
    write_data(0XE5);
    write_data(0XF6);
    write_data(0X17);
    write_data(0X77);
    write_data(0X77);
    write_data(0X77);
    write_data(0X77);
    write_data(0X77);
    write_data(0X77);
    write_data(0X71);

    write_command(0xB4); //Update Period Gate EQ Control in LPM 
    write_data(0X05); //LPM EQ Control 
    write_data(0X46);
    write_data(0X77);
    write_data(0X77);
    write_data(0X77);
    write_data(0X77);
    write_data(0X76);
    write_data(0X45);

    write_command(0x62); //Gate Timing Control
    write_data(0X32);
    write_data(0X03);
    write_data(0X1F);

    // write_command(0XC7);   //Ultra Low Power Mode  
    // write_data(0XC1);  
    // write_data(0X41);
    // write_data(0X26);

    write_command(0xB7); //Source EQ Enable 
    write_data(0X13);

    write_command(0XB0);   //Gate Line Setting
    write_data(0X64);  //400 line

    write_command(0x11); //Sleep out 
    sleep_ms(200);

    write_command(0xC9); //Source Voltage Select  
    write_data(0X00); //VSHP1; VSLP1 ; VSHN1 ; VSLN1

    write_command(0x36); //Memory Data Access Control
    // write_data(0X00); //Memory Data Access Control: MX=0 ; DO=0 
    write_data(0X48); //MX=1 ; DO=1 
    // write_data(0X4c); //MX=1 ; DO=1 GS=1

    write_command(0x3A); //Data Format Select 
    write_data(0X11); //10:4write for 24bit ; 11: 3write for 24bit

    write_command(0xB9); //Gamma Mode Setting 
    write_data(0X20); //20: Mono 00:4GS  

    write_command(0xB8); //Panel Setting 
    write_data(0x29); // Panel Setting: 0x29: 1-Dot inversion, Frame inversion, One Line Interlace

    //WRITE RAM 300X400
    write_command(0X2A);   //Column Address Setting
    write_data(0X05);
    write_data(0X36);
    write_command(0X2B);   //Row Address Setting
    write_data(0X00);
    write_data(0XC7);

    write_command(0x35); //TE
    write_data(0X00);

    write_command(0xD0); //Auto power dowb OFF
    // write_data(0X7F); //Auto power dowb OFF
    write_data(0XFF); //Auto power dowb ON


    write_command(0x39); //LPM:Low Power Mode ON
    // write_command(0x38); //HPM:high Power Mode ON


    write_command(0x29); //DISPLAY ON  
    // write_command(0x28); //DISPLAY OFF  

    // write_command(0x21); //Display Inversion On 
    write_command(0x20); //Display Inversion Off 

    write_command(0xBB); // Enable Clear RAM
    write_data(0x4F); // CLR=0 ; Enable Clear RAM,clear RAM to 0
}

void bsp_st7306_drawpoint(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= 300 || y >= 400) {
        return;
    }
    uint16_t byte_x = x / 2;
    uint16_t byte_y = y / 2;
    uint32_t byte_index = byte_y * (300 / 2) + byte_x;
    if (byte_index >= BSP_ST7306_BUFFER_SIZE) {
        return;
    }
    uint8_t one_two = y % 2;
    uint8_t line_bit_high = (x % 2) * 4;
    uint8_t line_bit_low = line_bit_high + 2;
    uint8_t write_bit_high_pos = 7 - (line_bit_high + one_two);
    uint8_t write_bit_low_pos = 7 - (line_bit_low + one_two);

    bool data_bit1 = (color & 0x02) >> 1;
    bool data_bit0 = (color & 0x01);

    if (data_bit1) {
        bsp_st7306_screen_buffer[byte_index] |= (1 << write_bit_high_pos);
    } else {
        bsp_st7306_screen_buffer[byte_index] &= ~(1 << write_bit_high_pos);
    }

    if (data_bit0) {
        bsp_st7306_screen_buffer[byte_index] |= (1 << write_bit_low_pos);
    } else {
        bsp_st7306_screen_buffer[byte_index] &= ~(1 << write_bit_low_pos);
    }
}

void bsp_st7306_drawline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;
    int e2;
    while (1) {
        bsp_st7306_drawpoint(x0, y0, color);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void bsp_st7306_update() {
#if BSP_ST7306_USE_DMA
    dma_channel_wait_for_finish_blocking(st7306_dma_channel);
#endif
    write_command(0x2A);//Column Address Setting S61~S182
    write_data(0X05);
    write_data(0X36); // 0X24-0X17=14 // 14*4*3=168
    write_command(0x2B);//Row Address Setting G1~G250
    write_data(0X00);
    write_data(0XC7); // 192*2=384
    write_command(0x2C);   //write image data

    dc_data();
    cs_select();
#if BSP_ST7306_USE_DMA
    dma_channel_configure(
            st7306_dma_channel,
            &st7306_dma_cfg,
            &spi_get_hw(spi_default)->dr, // write address spi data register
            bsp_st7306_screen_buffer, // read addr st7306 frame buffer
            BSP_ST7306_BUFFER_SIZE, // word count
            true
    );
#else
    spi_write_blocking(spi_default, bsp_st7306_screen_buffer, BUFFER_SIZE);
    cs_deselect();
#endif
}

#if BSP_ST7306_USE_DMA

void bsp_st7306_wait_for_dma_finish() {
    dma_channel_wait_for_finish_blocking(st7306_dma_channel);
    while (spi_is_busy(spi_default));
    cs_deselect();
}

#endif

void bsp_st7306_fill(uint16_t color) {
    if (color == BSP_ST7306_COLOR_BLACK) {
        memset(bsp_st7306_screen_buffer, 0xFF, BSP_ST7306_BUFFER_SIZE);
    } else {
        memset(bsp_st7306_screen_buffer, 0x00, BSP_ST7306_BUFFER_SIZE);
    }
}