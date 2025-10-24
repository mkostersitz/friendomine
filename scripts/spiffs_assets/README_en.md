# SPIFFS Assets Builder

This script builds the SPIFFS assets partition for ESP32 projects, packaging various resources into a device-usable format.

## Features

- Process WakeNet models
- Include text font files
- Process emoji image collections
- Auto-generate an assets index
- Package the final `assets.bin`

## Requirements

- Python 3.6+
- Required resource files

## Usage

### Basic syntax

```bash
./build.py --wakenet_model <wakenet_model_dir> \
    --text_font <text_font_file> \
    --emoji_collection <emoji_collection_dir>
```

### Arguments

| Option | Type | Required | Description |
|-------|------|----------|-------------|
| `--wakenet_model` | Directory | No | Path to the WakeNet model directory |
| `--text_font` | File | No | Path to the text font file |
| `--emoji_collection` | Directory | No | Path to the emoji image collection |

### Examples

```bash
# Full example
./build.py \
    --wakenet_model ../../managed_components/espressif__esp-sr/model/wakenet_model/wn9_nihaoxiaozhi_tts \
    --text_font ../../components/xiaozhi-fonts/build/font_puhui_common_20_4.bin \
    --emoji_collection ../../components/xiaozhi-fonts/build/emojis_64/

# Fonts only
./build.py --text_font ../../components/xiaozhi-fonts/build/font_puhui_common_20_4.bin

# Emojis only
./build.py --emoji_collection ../../components/xiaozhi-fonts/build/emojis_64/
```

## Workflow

1. Create build directories
   - `build/` - main build directory
   - `build/assets/` - resources directory
   - `build/output/` - output directory

2. Process WakeNet model
   - Copy model files into build directory
   - Run `pack_model.py` to generate `srmodels.bin`
   - Copy generated model file into assets

3. Process text font
   - Copy font file into assets
   - Supports `.bin` fonts

4. Process emoji collection
   - Scan images from the specified folder
   - Supports `.png` and `.gif`
   - Auto-generate emoji index

5. Generate config files
   - `index.json` - assets index
   - `config.json` - build config

6. Build final assets
   - Use `spiffs_assets_gen.py` to create `assets.bin`
   - Copy to build root

## Outputs

After a successful build, `build/` will contain:

- `assets/` - all resource files
- `assets.bin` - final SPIFFS bundle
- `config.json` - build configuration
- `output/` - intermediate outputs

## Supported formats

- Model files: `.bin` (via pack_model.py)
- Font files: `.bin`
- Images: `.png`, `.gif`
- Config: `.json`

## Error handling

The script includes robust error handling:

- Validate source files/directories
- Check subprocess results
- Provide detailed errors and warnings

## Notes

1. Ensure all dependent Python scripts are in the same directory
2. Use absolute paths or paths relative to this script
3. The build process cleans previous build artifacts
4. The size of `assets.bin` is limited by the SPIFFS partition size
