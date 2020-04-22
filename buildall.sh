#!/bin/bash

# docker pull multiarch/crossbuild

cleanup () {
    sudo git submodule foreach --recursive git clean -xfd
    sudo rm -rf build bin
}

copy_opengl_library () {
    mkdir dpf/build/
    cp $1 dpf/build/
}

rename_bin () {
    sudo chown -R $USER:$USER bin
    mv bin $1
}

# Linux
cleanup
make -C dpf/dgl
make -C src/gunshot/
# copy_opengl_library resources/linux/libdgl-opengl.a
# docker run -it --rm -v $(pwd):/workdir multiarch/crossbuild make -C src/gunshot/ HAVE_OPENGL=true
rename_bin linux

# # Windows 64 bit
# cleanup
# copy_opengl_library resources/win64/libdgl-opengl.a
# docker run -it --rm -v $(pwd):/workdir -e CROSS_TRIPLE=x86_64-w64-mingw32 multiarch/crossbuild make -C src/gunshot/
# sudo chown -R $USER:$USER bin
# mv bin win64
#
# # MacOS
# cleanup
# copy_opengl_library resources/macos/libdgl-opengl.a
# docker run -it --rm -v $(pwd):/workdir -e CROSS_TRIPLE=x86_64-apple-darwin multiarch/crossbuild make CXX=c++ CXXFLAGS=-stdlib=libc++ -C src/gunshot/
# sudo chown -R $USER:$USER bin
# mv bin macos

# # Clean up and zip
# zip release.zip -r linux win64 macos
# rm -rf linux win64 macos
# cleanup
