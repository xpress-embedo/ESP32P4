SET(SOURCES screens/ui_MainScreen.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_logo2_png.c)

add_library(ui ${SOURCES})
