# SKETCHKIT

A small utility thingie, to quickly make something using bgfx from c.
It uses RGFW as it windowing backend, HandMadeMath for math operations, qoi.h for image loading and m3d.h for basic model loading.

# Building
First, you'll need to build bgfx for your platform, drop the static libraries into the libs/ folder and drop bx/platform.h into the sk/ folder. Then:

- sh ./build.bat <myfile.c> [debug]
- cd shd
- sh ./build.bat 

On windows just use "build" instead of "sh ./build.bat".
For building the shaders shaderc must be visible in $PATH.
