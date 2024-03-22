FROM ghcr.io/autc04/retro68

COPY app /root/app
WORKDIR /root/app

RUN rm -rf build && mkdir build && cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake && \
    make
