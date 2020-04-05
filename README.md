# gunshot

Convolution VST plugin.

## Requirements

- Cross platform (Windows, OSX, and Linux).
- Impulse response must be stored with the plugin state (not just a path to a file as this may move independently of the project).
- Automatic sample-rate conversion of the impulse response recording.
- Parameters: Wet level (dB), dry level (dB). Possibly high-pass and low-pass filter (Hz).
- Minimalist GUI to add file browser. Parameters can be controlled without the GUI but may be added to the GUI for completeness.

## Libraries and Resources

- [DPF](https://github.com/DISTRHO/DPF) - DISTRHO Plugin Framework.
- [FFTConvolver](https://github.com/HiFi-LoFi/FFTConvolver) - Audio convolution algorithm in C++ for real time audio processing.
- [AudioFile](https://github.com/adamstark/AudioFile) - A simple C++ library for reading and writing audio files.
- [base64.c](https://github.com/joedf/base64.c) - Base64 Library in C
- [DejaVu Fonts](https://dejavu-fonts.github.io/) - The DejaVu fonts are a font family based on the Vera Fonts.

## Progress log

- Looking into modifying the KlangFalter instead of starting from scratch. KlangFalter is built using JUCE and I cannot get Projucer to build for linux.
- Found FFTConvolver which is the convolution library used in KlangFalter. It seems to be pure C++ and the test-program compiles just fine.
- Looking for a way to import WAV files.
- Figured out how to build DISTRHO's port of KlangFalter so I will try to modify this as it should build on all platforms.
- Could not work out how to modify KlangFalter - starting over instead.
- The state can now be extracted from the plugin when using Right Click - Save VST Preset. It is, however, not saved when storing a regular Bitwig preset. The Distrho States example exports the state in both cases (the Bitwig preset can be explored with `binwalk -> dd -> unzip`). There must be some little difference between the two...
- It seems like the state is only stored to a preset when the plugin has a UI - added a blank IO and now the preset looks good!
- I need a file browser. FLTK does not compile with `-fPIC` so it does not work well for shared objects. I am trying out some alternatives as FLTK is a bit hard to configure - nativefiledialog works very well! (only tested on linux but should work on the other platforms as well).
- Linux: The file browser crashes Reaper, Ardour, and Mixbus5. It works ok in Bitwig and Waveform Free. I should probably consider another file browser option. It seems like there is one built into DPF! ([gl-examples](https://github.com/DISTRHO/gl-examples/blob/master/examples/file-browser.cpp))
- The file browser now works well. Still missing: Parameters (wet, dry, low-pass, high-pass), sample rate conversion, and cross compilation.
- Cross compilation for Windows works somewhat. The GUI is fairly unstable and the font .

## Building

After cloning the repository, check out the submodules too:

    git submodule init
    git submodule update

The steps needed to build the plugin are:

    # Compiling for Linux
    make -C dpf/dgl
    make -C src/gunshot

    # Cross compiling for Windows from Linux/Ubuntu
    sudo apt install mingw-w64
    make WIN32=true CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ LD=x86_64-w64-mingw32-ld HAVE_CAIRO=false -C dpf/dgl
    make WIN32=true CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ LD=x86_64-w64-mingw32-ld HAVE_CAIRO=false -C src/gunshot
