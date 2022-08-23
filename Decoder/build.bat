mkdir /p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=D:\repos\emsdk\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake -G Ninja ../
ninja Decoderjs
