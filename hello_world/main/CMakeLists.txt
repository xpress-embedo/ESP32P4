# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

include(${CMAKE_CURRENT_LIST_DIR}/hello_world_lvgl/file_list_gen.cmake)

idf_component_register(
    SRCS main.c         # list the source files of this component
    gui_mng.c
    gui_mng_cfg.c
    # hello_world_lvgl/screens/main_gen.c
    # hello_world_lvgl/hello_world_lvgl_gen.c
    # hello_world_lvgl/ui.c
    # hello_world_lvgl/hello_world_lvgl.c
    # hello_world_lvgl/fonts/Inter_SemiBold_ttf_data.c
    INCLUDE_DIRS        # optional, add here public include directories
    "." "hello_world_lvgl" "hello_world_lvgl/screens" ${LV_DEMO_DIR}
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)

# Add the macro definition for this component
target_compile_definitions(${COMPONENT_LIB} PUBLIC LV_LVGL_H_INCLUDE_SIMPLE)
