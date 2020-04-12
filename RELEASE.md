# Release guide

## Update version number

The version number is found in `GunShot.cpp` and should be update in the commit containing the release.

## Disable logging

The `GUNSHOT_LOG_FILE` must not be defined in `log.h`.

## Tag the release

To tag and push version `X.Y.Z`, run the following commands:

    git tag vX.Y.Z
    git push origin vX.Y.Z

## Generate binaries

Check out the repo in a clean directory.

Linux:

    make -C dpf/dgl
    make -C src/gunshot

Windows (cross-compiled from Linux):

    sudo apt install mingw-w64
    make WIN32=true HAVE_CAIRO=false CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ LD=x86_64-w64-mingw32-ld -C dpf/dgl
    make WIN32=true HAVE_CAIRO=false CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ LD=x86_64-w64-mingw32-ld -C src/gunshot

Mac OSX:

    brew install pkg-config
    make -C dpf/dgl
    make -C src/gunshot


Rename the `bin` directory for each distribution and place them in a directory, `/tmp/release/` with the following structure:

    linux
        gunshot-dssi
        gunshot-dssi.so
        gunshot-ladspa.so
        gunshot.lv2
        gunshot.vst
        gunshot-vst.so
    osx
        gunshot.lv2
        gunshot.vst
    win
        gunshot.lv2
        gunshot-vst.dll

In `/tmp/release/`, run the following command to generate the archive for version `X.Y.Z`:

    zip gunshot-vX.Y.Z.zip -r ./*

Upload this to the Github release page.

## Change back the version number

The version number is found in `GunShot.cpp` should be 0.0.0 for all non-releases.
