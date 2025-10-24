# P3 Audio Conversion and Playback Tools

This folder contains Python scripts for working with audio in the P3 format.

## 1. Audio Conversion Tool (convert_audio_to_p3.py)

Converts a regular audio file into P3 format (4‑byte header + a stream of Opus packets) with optional loudness normalization.

### Usage

```bash
python convert_audio_to_p3.py <input-audio> <output.p3> [-l LUFS] [-d]
```

- `-l` sets the target loudness for normalization (default: -16 LUFS)
- `-d` disables loudness normalization

Consider using `-d` to disable normalization if any of the following are true:
- The audio is very short
- The audio has already been loudness-normalized
- The audio comes from the default TTS (the current default TTS used by XiaoZhi already targets -16 LUFS)

Example:
```bash
python convert_audio_to_p3.py input.mp3 output.p3
```

## 2. P3 Audio Player (play_p3.py)

Decodes and plays a P3 audio file.

### Features

- Decode and play P3 audio files
- Apply a short fade-out when playback ends or when interrupted to avoid clipping/pops
- Specify the file to play from the command line

### Usage

```bash
python play_p3.py <path-to.p3>
```

Example:
```bash
python play_p3.py output.p3
```

## 3. Convert P3 Back to Audio (convert_p3_to_audio.py)

Converts a P3 file back to a regular audio file.

### Usage

```bash
python convert_p3_to_audio.py <input.p3> <output-audio>
```

The output audio file must include an extension (e.g., .wav, .mp3).

Example:
```bash
python convert_p3_to_audio.py input.p3 output.wav
```

## 4. Batch Conversion GUI

A simple GUI tool for batch converting audio→P3 and P3→audio.

![GUI](./img/img.png)

### Usage
```bash
python batch_convert_gui.py
```

## Installing Dependencies

Install the required Python packages first:

```bash
pip install librosa opuslib numpy tqdm sounddevice pyloudnorm soundfile
```

Or use the provided requirements file:

```bash
pip install -r requirements.txt
```

## P3 Format Overview

P3 is a lightweight streaming audio format with the following structure:
- Each frame consists of a 4‑byte header followed by one Opus-encoded packet
- Header layout: [1 byte type, 1 byte reserved, 2 bytes length]
- Fixed sample rate: 16000 Hz, mono
- Frame duration: 60 ms
