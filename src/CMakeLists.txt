add_executable(${CMAKE_PROJECT_NAME})

set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/configs/lvgl/lv_conf.h CACHE PATH "Path to lv_conf.h")

add_subdirectory(lib/lvgl)
add_subdirectory(lib/cJSON)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        main.c
        bsp/bsp_st7306.c
        configs/lvgl/porting/lv_port_disp.c
        lib/ntp/ntp.c
        ui/custom/custom.c
        ui/generated/events_init.c
        ui/generated/gui_guider.c
        ui/generated/widgets_init.c
        ui/generated/setup_scr_screen.c
        ui/generated/guider_fonts/lv_font_FX_LED_140.c
        ui/generated/guider_fonts/lv_font_SourceHanSerifSC_Regular_22.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/cJSON
        ${CMAKE_CURRENT_SOURCE_DIR}/bsp
        ${CMAKE_CURRENT_SOURCE_DIR}/configs
        ${CMAKE_CURRENT_SOURCE_DIR}/configs/lvgl
        ${CMAKE_CURRENT_SOURCE_DIR}/configs/lvgl/porting
        ui/custom
        ui/generated
        ui/generated/guider_customer_fonts
        lib/ntp
)

# pull in common dependencies
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib hardware_spi hardware_dma pico_cyw43_arch_lwip_threadsafe_background lvgl::lvgl lvgl::demos cjson)

# set usb log output
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})
