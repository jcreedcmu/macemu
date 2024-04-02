FROM debian:unstable

ENV BUILD=/build

RUN mkdir $BUILD
WORKDIR $BUILD

# maybe some of these packages are not needed:
# ninja-build wget xz-utils curl
RUN apt-get update && \
    apt-get install -y git cmake clang ninja-build wget xz-utils libgmp-dev curl \
                       libmpfr-dev libmpc-dev libboost-all-dev bison flex texinfo ruby mlton

# Checkout Retro68
ADD https://api.github.com/repos/agoode/Retro68/git/refs/heads/mlton Retro68-version.json
RUN git clone --depth 1 --branch mlton https://github.com/agoode/Retro68.git
RUN cd Retro68 && git submodule update --init --recursive

# Build Retro68
RUN mkdir /Retro68-INSTALL && \
    mkdir Retro68-build && \
    cd Retro68-build && \
    ../Retro68/build-toolchain.bash \
		--prefix=/Retro68-INSTALL \
		--no-ppc \
		--no-carbon \
		--multiversal

# Make sure Retro68 toolchain is in PATH
ENV PATH="/Retro68-INSTALL/bin:${PATH}"

# build gmp library for m68k target
RUN wget https://gmplib.org/download/gmp/gmp-6.3.0.tar.xz && \
    tar xvf gmp-6.3.0.tar.xz
RUN cd $BUILD/gmp-6.3.0 && \
    ./configure --host=m68k-apple-macos \
		  --prefix=$BUILD/gmp-m68k-INSTALL && \
    make && make install

ADD https://api.github.com/repos/agoode/mlton/git/refs/heads/mac mlton-version.json
RUN git clone --depth 1 --branch mac https://github.com/agoode/mlton.git

# build mlton binary
RUN cd mlton && \
    export MLTON=$BUILD/mlton-INSTALL && \
    make all && \
    make PREFIX=$MLTON install

# make a symlink to account for the runtime build invoking cc rather than gcc
RUN ln -s /Retro68-INSTALL/bin/m68k-apple-macos-gcc /Retro68-INSTALL/bin/m68k-apple-macos-cc

# build m68k runtime
RUN cd mlton && \
    git clean -dfx && \
    make \
        USE_PREGEN=true \
		  TARGET_OS=macos \
		  TARGET_ARCH=m68k \
		  TARGET=m68k-apple-macos \
		  WITH_GMP_DIR=$BUILD/gmp-m68k-INSTALL \
		  PREFIX=$BUILD/mlton-INSTALL \
		  dirs runtime install-runtime

# patch the lack of existence of svnversion, needed for twelf build
RUN echo '#!/bin/bash\ngit rev-parse HEAD' > /usr/bin/svnversion && \
    chmod +x /usr/bin/svnversion

# check out agoode's twelf branch
ADD https://api.github.com/repos/agoode/twelf/git/refs/heads/mac twelf-version.json
RUN git clone --branch mac --depth 1 https://github.com/agoode/twelf

# build twelf library
RUN cd twelf && \
    make mlton="$BUILD/mlton-INSTALL/bin/mlton \
        -default-ann 'nonexhaustiveMatch ignore' \
        -default-ann 'nonexhaustiveBind ignore'" \
        classic-mac

# give more standard names
RUN mv /build/twelf/bin/twelf-server.a /build/twelf/bin/libtwelf.a && \
    mv /build/twelf/bin/twelf_server.h /build/twelf/bin/libtwelf.h

# install build script
COPY scripts/build.sh /root/build.sh
