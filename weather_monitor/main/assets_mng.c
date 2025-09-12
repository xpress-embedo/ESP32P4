/*
 * assets_mng.c
 *
 *  Created on: 10-Sep-2025
 *      Author: abc@xyz
 */

#include "assets_mng.h"

#include "esp_log.h"
#include "esp_littlefs.h"

// Private Variables
static const char *TAG = "ASSETS";

// Public Function Definitions
/**
 * @brief Mount the LittleFS filesystem for assets
 * @param  none
 * @return none
 */
void assets_mng_init( void )
{
  const esp_vfs_littlefs_conf_t conf = 
  {
    .base_path = "/assets",
    .partition_label = "assets",
    .format_if_mount_failed = false,  // Set to true if you want auto-format on failure
    .dont_mount = false,
  };

  esp_err_t err = esp_vfs_littlefs_register( &conf );
  if ( err != ESP_OK )
  {
    if ( err == ESP_FAIL )
    {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } 
    else if ( err == ESP_ERR_NOT_FOUND )
    {
      ESP_LOGE(TAG, "Failed to find LittleFS partition");
    } 
    else 
    {
      ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(err));
    }
    return;
  }

  size_t total = 0, used = 0;
  err = esp_littlefs_info( conf.partition_label, &total, &used );
  if ( err == ESP_OK )
  {
    ESP_LOGI(TAG, "LittleFS mounted: total=%d bytes, used=%d bytes", total, used);
  }
  else 
  {
    ESP_LOGW(TAG, "Failed to get LittleFS info (%s)", esp_err_to_name(err));
    return;
  }

  // Test Code to read a text file from the mounted filesystem
  FILE *f = fopen("/assets/hello.txt", "r");
  if ( f )
  {
    char buf[64];
    fgets(buf, sizeof(buf), f);
    fclose(f);
    ESP_LOGW(TAG, "Read from hello.txt: %s", buf);
  }
  else 
  {
    ESP_LOGE(TAG, "Failed to open hello.txt");
  }
  // Test Code Ends
}

/**
 * @brief 
 * @param path 
 * @param img_dsc 
 * @return 
 */
bool assets_mng_load_image( const char *path, lv_img_dsc_t *img_dsc )
{
  // opens the image file in binary mode for reading
  FILE *f = fopen(path, "rb");
  if ( !f )
  {
    ESP_LOGE("LVGL", "Failed to open image file: %s", path);
    return false;
  }

  // calculates the file size by seeking to the end and rewinding
  fseek( f, 0, SEEK_END );
  size_t size = ftell( f );
  rewind(f);

  // allocates memory for the image data
  uint8_t *buffer = malloc(size);
  if ( !buffer )
  {
    fclose(f);
    ESP_LOGE("LVGL", "Failed to allocate memory");
    return false;
  }

  // reads the file data into the buffer and closes the file
  fread( buffer, 1, size, f );
  fclose(f);

  // copies the image descriptor (metadata like width, height, color format) 
  // from the start of the buffer
  memcpy( img_dsc, buffer, sizeof(lv_img_dsc_t) );
  // Points img_dsc->data to the pixel data portion of the buffer
  img_dsc->data = buffer + sizeof(lv_img_dsc_t);
  return true;
}

void assets_mng_unload_image( lv_obj_t *img, lv_img_dsc_t *img_dsc )
{
  if ( img )
  {
    lv_obj_del( img );
  }
  else
  {
    ESP_LOGW(TAG, "Image object is NULL");
  }

  if ( img_dsc && img_dsc->data )
  {
    void *original_ptr = (void *)( img_dsc->data - sizeof(lv_img_dsc_t) );
    free( original_ptr );
    img_dsc->data = NULL;
  }
  else
  {
    ESP_LOGW(TAG, "Image data is NULL");
  }

  if ( img_dsc == NULL )
  {
    memset( img_dsc, 0, sizeof(lv_img_dsc_t) );
  }
  else
  {
    ESP_LOGW(TAG, "Image descriptor is NULL");
  }
}
