// image_to_jpeg.h - Efficient image to JPEG conversion interface
// JPEG encoding implementation that saves approximately 8KB of SRAM
#pragma once
#include "sdkconfig.h"
#ifndef CONFIG_IDF_TARGET_ESP32

#include <stdint.h>
#include <stddef.h>
#include <linux/videodev2.h>

typedef uint32_t v4l2_pix_fmt_t; // see linux/videodev2.h for details

#ifdef __cplusplus
extern "C" {
#endif

// JPEG output callback function type
// arg: User-defined parameter, index: Current data index, data: JPEG data block, len: Data block length
// Returns: Number of bytes actually processed
typedef size_t (*jpg_out_cb)(void *arg, size_t index, const void *data, size_t len);

/**
 * @brief Efficiently convert image formats to JPEG
 * 
 * This function uses an optimized JPEG encoder with the following features:
 * - Saves approximately 8KB of SRAM (static variables moved to heap)
 * - Supports multiple input image formats
 * - High-quality JPEG output
 * 
 * @param src       Source image data
 * @param src_len   Length of source image data
 * @param width     Image width
 * @param height    Image height
 * @param format    Image format (V4L2_PIX_FMT_RGB565, V4L2_PIX_FMT_RGB24, etc.)
 * @param quality   JPEG quality (1-100)
 * @param out       Output JPEG data pointer (caller must free)
 * @param out_len   Output JPEG data length
 * 
 * @return true on success, false on failure
 */
bool image_to_jpeg(uint8_t *src, size_t src_len, uint16_t width, uint16_t height, 
                   v4l2_pix_fmt_t format, uint8_t quality, uint8_t **out, size_t *out_len);

/**
 * @brief Convert image format to JPEG (callback version)
 * 
 * Uses a callback to handle JPEG output, suitable for streaming or chunked processing:
 * - Saves approximately 8KB of SRAM (static variables moved to heap)
 * - Supports streaming output without preallocating a large buffer
 * - Processes JPEG data in chunks via callback
 * 
 * @param src       Source image data
 * @param src_len   Length of source image data
 * @param width     Image width
 * @param height    Image height
 * @param format    Image format
 * @param quality   JPEG quality (1-100)
 * @param cb        Output callback function
 * @param arg       User argument passed to callback
 * 
 * @return true on success, false on failure
 */
bool image_to_jpeg_cb(uint8_t *src, size_t src_len, uint16_t width, uint16_t height, 
                      v4l2_pix_fmt_t format, uint8_t quality, jpg_out_cb cb, void *arg);

#ifdef __cplusplus
}
#endif

#endif // ndef CONFIG_IDF_TARGET_ESP32
