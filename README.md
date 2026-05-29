#SimpleMeters

<img width="1874" height="750" alt="image" src="https://github.com/user-attachments/assets/5a84ee06-4f50-4896-b887-696275f495fd" />

SimpleMeters is an audio analysis tool for the Windows platform.

> [!NOTE]
> This project is currently under development and is in an early stage.

# Development Progress

Features currently being developed or already completed include:

- [ ] Configuration file
- [ ] Component position configuration
- [ ] Component size configuration
- [ ] In-component text input on click
- [ ] Component hover effect
- [ ] Component pause functionality
- [ ] VST3 support
- [ ] Spectrum mode switching
- [ ] Spectrum display with gain curve
- [ ] Spectrum coordinate mapping curve
- [ ] Spectrum bar graph
- [ ] Spectrum waterfall display
- [ ] Spectrum reference lines
- [ ] Spectrum fundamental frequency / peak display
- [ ] True peak display
- [ ] Loudness metering (LUFS, RMS, etc.)
- [ ] Select different sample buffers for volume analysis
- [ ] Waveform color changes based on left/right channel frequency band energy
- [ ] Adjustable waveform scroll speed
- [ ] Optimize waveform ring buffer
- [ ] Chart range selection
- [ ] Chart range selection based on fundamental frequency
- [ ] Waveform reference lines
- [ ] Waveform L-R
- [ ] Stereo analysis mode
- [ ] Stereo analysis graph rotation
- [ ] Stereo analysis graph color change over time
- [ ] Stereo analysis reference lines
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
