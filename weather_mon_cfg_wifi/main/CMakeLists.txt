# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c         # list the source files of this component
    dht11.c
    influxDB.c
    gui_mng.c
    gui_mng_cfg.c
    data_config.c
    assets_mng.c
    # nvs_mng.c
    wifi_sta_mng.c
    ui/screens/ui_Screen1.c
    ui/screens/ui_Screen2.c
    ui/ui.c
    ui/components/ui_comp_hook.c
    ui/ui_helpers.c
    ui/images/ui_img_wifi_png.c
    ui/images/ui_img_wifi_disconnected_png.c
    ui/images/ui_img_wifi_connected_png.c
    ui/fonts/ui_font_Number120.c
    INCLUDE_DIRS        # optional, add here public include directories
    "." "ui" "ui/screens" ${LV_DEMO_DIR}
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)

# Add the macro definition for this component
target_compile_definitions(${COMPONENT_LIB} PUBLIC LV_LVGL_H_INCLUDE_SIMPLE)

