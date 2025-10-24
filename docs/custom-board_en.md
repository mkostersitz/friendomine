# Custom Board Guide

This guide explains how to add a new board initialization for the Xiaozhi AI voice chat project. Xiaozhi supports 70+ ESP32-series boards, each with its own initialization code under the matching directory.

## Important Notes

> Warning: For custom boards, if your IO mapping differs from an existing board, do NOT overwrite and build the existing board config. You must create a new board type, or define a new name and sdkconfig macros via the builds section of config.json to distinguish it. Use `python scripts/release.py [board-folder-name]` to build and package firmware.
>
> If you overwrite an existing config, future OTA updates may replace your custom firmware with the standard firmware for that board, causing malfunction. Each board has a unique identifier and its own firmware update channel. Keeping the board identity unique is critical.

## Directory Structure

Each board directory typically contains:

- `xxx_board.cc` - main board initialization code
- `config.h` - board configuration, pin mapping and other settings
- `config.json` - build configuration, target chip and special options
- `README.md` - board notes and instructions

## Steps to Customize a Board

### 1. Create a new board directory

Under `boards/`, create a new directory named `[brand]-[board-type]`, for example `m5stack-tab5`:

```bash
mkdir main/boards/my-custom-board
```

### 2. Create configuration files

#### config.h

Define all hardware settings in `config.h`, including:

- Audio sample rates and I2S pins
- Audio codec address and I2C pins
- Button and LED pins
- Display parameters and pins

Sample (from lichuang-c3-dev):

```c
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// Audio
#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_10
#define AUDIO_I2S_GPIO_WS   GPIO_NUM_12
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_8
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_7
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_11

#define AUDIO_CODEC_PA_PIN       GPIO_NUM_13
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_0
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_1
#define AUDIO_CODEC_ES8311_ADDR  ES8311_CODEC_DEFAULT_ADDR

// Button
#define BOOT_BUTTON_GPIO        GPIO_NUM_9

// Display
#define DISPLAY_SPI_SCK_PIN     GPIO_NUM_3
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_5
#define DISPLAY_DC_PIN          GPIO_NUM_6
#define DISPLAY_SPI_CS_PIN      GPIO_NUM_4

#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY true

#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_2
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT true

#endif // _BOARD_CONFIG_H_
```

#### config.json

Define build settings in `config.json`. The `scripts/release.py` tool uses this to automate builds:

```json
{
    "target": "esp32s3",  // Target chip model: esp32, esp32s3, esp32c3, esp32c6, esp32p4, etc.
    "builds": [
        {
            "name": "my-custom-board",  // Board name, used in firmware artifacts
            "sdkconfig_append": [
                // Flash size settings
                "CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y",
                // Partition table file
                "CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions/v2/8m.csv\""
            ]
        }
    ]
}
```

Field notes:
- target: target chip model, must match hardware
- name: output firmware package name, recommended to match directory name
- sdkconfig_append: extra sdkconfig entries appended to defaults

Common sdkconfig_append snippets:
```json
// Flash size
"CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y"   // 4MB Flash
"CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y"   // 8MB Flash
"CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y"  // 16MB Flash

// Partition tables
"CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions/v2/4m.csv\""  // 4MB partitions
"CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions/v2/8m.csv\""  // 8MB partitions
"CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions/v2/16m.csv\"" // 16MB partitions

// Language options
"CONFIG_LANGUAGE_EN_US=y"  // English
"CONFIG_LANGUAGE_ZH_CN=y"  // Simplified Chinese

// Wake word options
"CONFIG_USE_DEVICE_AEC=y"          // Enable device-side AEC
"CONFIG_WAKE_WORD_DISABLED=y"      // Disable wake word
```

### 3. Implement board initialization code

Create a `my_custom_board.cc` and implement all board init logic.

A basic board class includes:

1. Class definition: derive from `WifiBoard` or `Ml307Board`
2. Initialization: I2C, display, buttons, IoT and other components
3. Virtual overrides: e.g., `GetAudioCodec()`, `GetDisplay()`, `GetBacklight()`
4. Board registration: register using `DECLARE_BOARD`

```cpp
#include "wifi_board.h"
#include "codecs/es8311_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "mcp_server.h"

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>

#define TAG "MyCustomBoard"

class MyCustomBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_;
    Button boot_button_;
    LcdDisplay* display_;

    // I2C initialization
    void InitializeI2c() {
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }

    // SPI initialization (for display)
    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SPI_SCK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    // Button initialization
    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
    }

    // Display initialization (ST7789 example)
    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;
        
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_SPI_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_DC_PIN;
        io_config.spi_mode = 2;
        io_config.pclk_hz = 80 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));
        
        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, true);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        
        // Create display object
        display_ = new SpiLcdDisplay(panel_io, panel,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                                    DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                                    DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    // MCP Tools initialization
    void InitializeTools() {
        // See MCP docs
    }

public:
    // Constructor
    MyCustomBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeI2c();
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
        InitializeTools();
        GetBacklight()->SetBrightness(100);
    }

    // Audio codec accessor
    virtual AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec audio_codec(
            codec_i2c_bus_, 
            I2C_NUM_0, 
            AUDIO_INPUT_SAMPLE_RATE, 
            AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, 
            AUDIO_I2S_GPIO_BCLK, 
            AUDIO_I2S_GPIO_WS, 
            AUDIO_I2S_GPIO_DOUT, 
            AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, 
            AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }

    // Display accessor
    virtual Display* GetDisplay() override {
        return display_;
    }
    
    // Backlight accessor
    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }
};

// Register board
DECLARE_BOARD(MyCustomBoard);
```

### 4. Add build system config

#### Add a board option in Kconfig.projbuild

Open `main/Kconfig.projbuild` and add a new item under `choice BOARD_TYPE`:

```kconfig
choice BOARD_TYPE
    prompt "Board Type"
    default BOARD_TYPE_BREAD_COMPACT_WIFI
    help
        Board type
    
    # ... other boards ...
    
    config BOARD_TYPE_MY_CUSTOM_BOARD
        bool "My Custom Board"
        depends on IDF_TARGET_ESP32S3  # Pick the matching target for your board
endchoice
```

Notes:
- BOARD_TYPE_MY_CUSTOM_BOARD is the config symbol; use uppercase with underscores
- depends on should match the target chip (IDF_TARGET_ESP32S3, IDF_TARGET_ESP32C3, etc.)
- Description can be English-only

#### Add the board case to CMakeLists.txt

Open `main/CMakeLists.txt` and add a new case in the board selection chain:

```cmake
# Add your board here in the elseif chain
elseif(CONFIG_BOARD_TYPE_MY_CUSTOM_BOARD)
    set(BOARD_TYPE "my-custom-board")  # Match directory name
    set(BUILTIN_TEXT_FONT font_puhui_basic_20_4)  # Choose a proper font for your screen size
    set(BUILTIN_ICON_FONT font_awesome_20_4)
    set(DEFAULT_EMOJI_COLLECTION twemoji_64)  # Optional, if emoji display is needed
endif()
```

Fonts and emoji notes:

Choose a font size based on resolution:
- Small screens (128x64 OLED): `font_puhui_basic_14_1` / `font_awesome_14_1`
- Small/medium (240x240): `font_puhui_basic_16_4` / `font_awesome_16_4`
- Medium (240x320): `font_puhui_basic_20_4` / `font_awesome_20_4`
- Large (480x320+): `font_puhui_basic_30_4` / `font_awesome_30_4`

Emoji collections:
- `twemoji_32` - 32x32 pixels (small screens)
- `twemoji_64` - 64x64 pixels (bigger screens)

### 5. Configure and build

#### Method A: Configure with idf.py

1. Set target chip (first time or when changing targets):
   ```bash
   # For ESP32-S3
   idf.py set-target esp32s3
   
   # For ESP32-C3
   idf.py set-target esp32c3
   
   # For ESP32
   idf.py set-target esp32
   ```

2. Clean prior configuration:
   ```bash
   idf.py fullclean
   ```

3. Open menuconfig:
   ```bash
   idf.py menuconfig
   ```
   Navigate to: `Xiaozhi Assistant` -> `Board Type` and select your custom board.

4. Build and flash:
   ```bash
   idf.py build
   idf.py flash monitor
   ```

#### Method B: Use release.py (recommended)

If your board directory has a `config.json`, you can automate:

```bash
python scripts/release.py my-custom-board
```

This script will:
- Set the target based on `config.json` -> `target`
- Apply options in `sdkconfig_append`
- Build and package firmware

### 6. Create README.md

Document the board features, hardware requirements, and build/flash steps in README.md.


## Common board components

### 1. Display

Supported displays include:
- ST7789 (SPI)
- ILI9341 (SPI)
- SH8601 (QSPI)
- etc.

### 2. Audio codecs

Supported codecs include:
- ES8311 (common)
- ES7210 (mic array)
- AW88298 (amplifier)
- etc.

### 3. Power management

Some boards use PMICs:
- AXP2101
- other available PMICs

### 4. MCP device tools

You can add various MCP tools so the AI can operate:
- Speaker (speaker control)
- Screen (screen brightness)
- Battery (battery level)
- Light (lighting control)
- etc.

## Board class hierarchy

- `Board` - base class
  - `WifiBoard` - Wi-Fi boards
  - `Ml307Board` - 4G module boards
  - `DualNetworkBoard` - switchable Wi‑Fi/4G boards

## Tips

1. Refer to similar boards: if your board is similar to an existing one, start from it
2. Debug step-by-step: bring up basic parts (display) first, then add audio, etc.
3. Pin mapping: ensure all pins are correctly configured in config.h
4. Check compatibility: verify chips and drivers are compatible

## Troubleshooting

1. Display issues: check SPI config, mirror/swap, and color inversion
2. No audio output: check I2S settings, PA enable pin, and codec address
3. Network not connected: check Wi‑Fi credentials and network config
4. No server communication: check MQTT or WebSocket settings

## References

- ESP-IDF: https://docs.espressif.com/projects/esp-idf/
- LVGL: https://docs.lvgl.io/
- ESP-SR: https://github.com/espressif/esp-sr
