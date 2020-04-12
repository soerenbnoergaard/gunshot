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
- [libsamplerate](https://github.com/erikd/libsamplerate) - libsamplerate (also known as Secret Rabbit Code) is a library for performing sample rate conversion of audio data.

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
- Cross compilation for Windows works somewhat. The GUI is fairly unstable and the font. The drawn GUI seems to stay shown after closing the window (the same seems to be true for the DPF example plugins). Maybe it is related to the cross compilation technique?
- The Dragonfly reverb also has the same artfacts (i.e. GUI not closing and fonts not showing) as I do - both when I compile it myself and when I download release 3.0. Maybe it is related to testing with VirtualBox? It may not handle OpenGL so well...
- Implemented sample rate conversion - this was very easy using libsamplerate! Sample rate conversion is done at run-time (and not when the sample is loaded) in order to adapt to changes in sample rate in a project.
- Cross compilation does work on a real Windows PC (not a VirtaulBox). The issue must have been related to the graphics drivers in VirtualBox. However, the plugin crashes Windows Tracktion T7, Mixbus, and Reaper when saving state. Maybe something is wrong with `getState()`?
- Fixed the bug where the plugin crashes in Windows DAWs. There was problem with dynamic array allocation in the `String` class of `dpf`. I have created a fork with the changes and will send a pull request.
- Looking at cross compiling using this Docker image: [multiarch/crossbuild](https://hub.docker.com/r/multiarch/crossbuild). There are some issues with the OpenGL libraries. However, I can compile on a Mac as well.
- In Linux Bitwig: Terminate process and reload with several instances of `gunshot`, all plugins crash when re-activating the engine. Probably related to all the heavy calculations taking place in the real-time section (`GunShot::run`). This has been improved a lot by moving the hard work out of `run()`, but the plugin can still crash sometimes when loading a new impulse response during playback. Perhalps some work could be done in a separate thread? I believe this is fixed now as I have not seen it recently.
- In Windows Reaper and Windows Mixbus, the plugin starts with no initial impulse response (not even a dirac). Maybe this is fixed now (there was a missing step in `initState()`). This has been solved.
- Added parameters for high-pass, low-pass, dry level, and wet level.

## Bugs and TODO

- The CPU usage is worse than Klangfalter. This is probably because KlangFalter does background processing of the tail section using the `TwoStageFFTConvolver`.
- KlangFalter has adoptive head and tail block sizes. The head the buffer size or the smallest factor of two larger than this. The tail block size is the largest of 8192 and two times the head block size.

## Building

After cloning the repository, check out the submodules too:

    git submodule init
    git submodule update

    # To clean a repo already checked out
    git clean -xfd
    git submodule foreach --recursive git clean -xfd
    git reset --hard
    git submodule foreach --recursive git reset --hard
    git submodule update --init --recursive

The steps needed to build the plugin are:

    # Compiling for Linux
    make -C dpf/dgl
    make -C src/gunshot

    # Cross compiling for Windows from Linux/Ubuntu
    sudo apt install mingw-w64
    make WIN32=true HAVE_CAIRO=false CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ LD=x86_64-w64-mingw32-ld -C dpf/dgl
    make WIN32=true HAVE_CAIRO=false CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ LD=x86_64-w64-mingw32-ld -C src/gunshot

    # Compiling on Mac OSX
    brew install pkg-config
    make -C dpf/dgl
    make -C src/gunshot
