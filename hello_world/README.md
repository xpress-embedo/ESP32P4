Hello World
====================

The following is the website which we are using to add several components to our project.  
[Espressif Component Registry Website](https://components.espressif.com/)

The following are the components/dependencies we have added to our base project from Component Registry website.
* `esp32_p4_function_ev_board` : This component consist of board support package for this development board.
* `esp_lcd_ek79007` : This development board has the LCD `EK79007`, and this component contains the display drivers for the same.
* `esp_lcd_ili9881c` : This component contains the display drivers for the `ILI9881C` as of now I am not sure why this is added in the example project, if not required this will be removed.
* `esp_lcd_touch` : This component contains the generic touch handling.
* `esp_lcd_touch_gt911` : This contains the touch drivers for GT911
* `esp_lvgl_port` : This component contains the ESP LVGL port
* `lvgl` : This component contains the LVGL graphics library.
