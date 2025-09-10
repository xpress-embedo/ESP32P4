LVGL Image Converter Tool
=========================
This is the LVGL official Python based Image Converter Tool, which is downloaded from the following GitHub link.  
[LVGLImage.py](https://github.com/lvgl/lvgl/blob/master/scripts/LVGLImage.py)

There is also a web based tool as given below.  
[LVGL Web Based Image Converter Tool](https://lvgl.io/tools/imageconverter)

Refer below option for image conversion.
```bash
usage: LVGLImage.py [-h] [--ofmt {C,BIN,PNG}]
                    [--cf {L8,I1,I2,I4,I8,A1,A2,A4,A8,ARGB8888,XRGB8888,RGB565,RGB565_SWAPPED,RGB565A8,ARGB8565,RGB888,AUTO,RAW,RAW_ALPHA,ARGB8888_PREMULTIPLIED}]
                    [--rgb565dither] [--premultiply]
                    [--compress {NONE,RLE,LZ4}] [--align [byte]]
                    [--background [color]] [--nemagfx] [-o OUTPUT]
                    [--name NAME] [-v]
                    input
```
* `[--ofmt {C,BIN,PNG}]` : Output Format of the file.
* `[--cf {L8,I1,I2,I4,I8,A1,A2,A4,A8,ARGB8888,XRGB8888,RGB565,RGB565_SWAPPED,RGB565A8,ARGB8565,RGB888,AUTO,RAW,RAW_ALPHA,ARGB8888_PREMULTIPLIED}]` : This is the color format of the output file, normally for ESP32P4 Function Evaluation Board the supported color is `RGB565`
* `[--rgb565dither]` : TODO
* `[--premultiply]` : TODO
* `[--compress {NONE,RLE,LZ4}]` : This is compression technique, if compression technique is used the compression decoder will be used, so used as per requirement.
* `[--align [byte]]` : TODO
* `[--background [color]]` : TODO
* `[--nemagfx]` : TODO
* `[-o OUTPUT]` : Output file name
* `[--name NAME]` : Image Variable Name
* `input` : Input Image

The following are the some of the commands to convert images.  
```bash
python LVGLImage.py --ofmt C --cf RGB565 --name ui_img_logo1_png -o ui_img_logo1_png.c logo1.png
```
The above command, converts the `logo1.png` image into `ui_img_logo1_png.c` file, with the same variable name.

```bash
python LVGLImage.py --ofmt C --cf RGB565 --compress RLE --name ui_img_logo1_png -o ui_img_logo1_png.c logo1.png
```
The above command, converts the `logo1.png` image into `ui_img_logo1_png.c` file, with the same variable name, and compress the image using RLE algorithm.

```bash
python LVGLImage.py --ofmt BIN --cf RGB565 --name ui_img_main_logo -o ui_img_main_logo.bin logo1.png
```
The above command, converts the `logo1.png` image into `ui_img_main_logo.bin` file i.e. the binary file, with the same variable name.