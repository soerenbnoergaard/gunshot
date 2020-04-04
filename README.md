# gunshot

Convolution VST plugin.

## Requirements

- Cross platform (Windows, OSX, and Linux).
- Impulse response must be stored with the plugin state (not just a path to a file as this may move independently of the project).
- Automatic sample-rate conversion of the impulse response recording.
- Parameters: Wet level (dB), dry level (dB). Possibly high-pass and low-pass filter (Hz).
- Minimalist GUI to add file browser. Parameters can be controlled without the GUI but may be added to the GUI for completeness.

## Libraries

- [DPF](https://github.com/DISTRHO/DPF) - DISTRHO Plugin Framework.
- [FFTConvolver](https://github.com/HiFi-LoFi/FFTConvolver) - Audio convolution algorithm in C++ for real time audio processing.
- [AudioFile](https://github.com/adamstark/AudioFile) - A simple C++ library for reading and writing audio files.

## Progress log

- Looking into modifying the KlangFalter instead of starting from scratch. KlangFalter is built using JUCE and I cannot get Projucer to build for linux.
- Found FFTConvolver which is the convolution library used in KlangFalter. It seems to be pure C++ and the test-program compiles just fine.
- Looking for a way to import WAV files.
- Figured out how to build DISTRHO's port of KlangFalter so I will try to modify this as it should build on all platforms.
- Could not work out how to modify KlangFalter - starting over instead.
