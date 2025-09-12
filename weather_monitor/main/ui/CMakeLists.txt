SET(SOURCES screens/ui_Screen1.c
    screens/ui_Screen2.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_wifi_disconnected_png.c
    images/ui_img_wifi_connected_png.c
    images/ui_img_logo1_png.c
    images/ui_img_logo2_png.c
    images/ui_img_logo3_png.c
    images/ui_img_wifi_png.c
    fonts/ui_font_Number120.c)

add_library(ui ${SOURCES})
