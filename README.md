# SimpleMeters

[中文](README_zh.md)

<img width="2550" height="600" alt="屏幕截图 2026-07-02 222352" src="https://github.com/user-attachments/assets/4e2ae00c-5d73-4a2e-bd5c-38cca32ef571" />

SimpleMeters is an audio analysis tool for the Windows platform.

> [!NOTE]
> You can use the basic features now, but most of the features haven't been added yet.

# Development Progress

Features currently being developed or already completed include:

- [x] Configuration file
- [x] Component position configuration
- [x] Component size configuration
- [ ] In-component text input on click
- [ ] Component hover effect
- [ ] Component pause functionality
- [ ] VST3 support
- [x] Spectrum mode switching
- [ ] Spectrum display with gain curve
- [ ] Spectrum coordinate mapping curve
- [ ] Spectrum bar graph
- [ ] Spectrum waterfall display
- [x] Spectrum reference lines
- [x] Spectrum fundamental frequency / peak display
- [ ] True peak display
- [ ] Loudness metering (LUFS, RMS, etc.)
- [ ] Select different sample buffers for volume analysis
- [ ] Waveform color changes based on left/right channel frequency band energy
- [ ] Adjustable waveform scroll speed
- [ ] Optimize waveform ring buffer
- [ ] Chart range selection
- [x] Chart range selection based on fundamental frequency
- [x] Waveform reference lines
- [x] Waveform L-R
- [ ] Stereo analysis mode
- [ ] Stereo analysis graph rotation
- [ ] Stereo analysis graph color change over time
- [x] Stereo analysis reference lines
- [ ] BPM blinking
- [ ] Pre-gain
- [ ] Audio input detection
- [ ] Other components

New features are generally added to the list above.

Already implemented features will be placed in documentation, describing their functionality and specific implementation. Currently, no documentation has been created.

# Installation

No installation is required at all, as the project only consists of a standalone .exe and a VST3 file (planned).

# Usage
- For the released .exe file:
  - In the top-left corner, turn WASAPI loopback on/off. Right-click inside the window to select the corresponding component for analysis.

# Project Configuration

A `.jucer` file is located in the project root directory.  
After opening the `.jucer` file with Projucer, specify the JUCE library root directory in the global settings to ensure all modules are referenced correctly.

# Acknowledgements 💕

[JUCE](https://github.com/juce-framework/JUCE)  
Provides the basic framework.

[miniaudio](https://github.com/mackron/miniaudio)  
Provides WASAPI loopback on Windows.
