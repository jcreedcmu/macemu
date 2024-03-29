FROM ubuntu:latest

ENV BUILD=/build

RUN mkdir $BUILD
WORKDIR $BUILD

# maybe some of these packages are not needed:
# ninja-build wget xz-utils curl
RUN apt-get update && \
    apt-get install -y git cmake clang ninja-build wget xz-utils libgmp-dev curl \
                       libmpfr-dev libmpc-dev libboost-all-dev bison flex texinfo ruby

ADD https://api.github.com/repos/agoode/Retro68/git/refs/heads/mlton Retro68-version.json
RUN git clone --depth 1 --branch mlton https://github.com/agoode/Retro68.git

RUN cd Retro68 && git submodule update --init --recursive

RUN mkdir /Retro68-INSTALL && \
    mkdir Retro68-build && \
    cd Retro68-build && \
    ../Retro68/build-toolchain.bash \
		--prefix=/Retro68-INSTALL \
		--no-ppc \
		--no-carbon \
		--multiversal

ENV PATH="/Retro68-INSTALL/bin:${PATH}"

# build gmp library for m68k target
RUN wget https://gmplib.org/download/gmp/gmp-6.3.0.tar.xz && \
    tar xvf gmp-6.3.0.tar.xz
RUN cd $BUILD/gmp-6.3.0 && \
    ./configure --host=m68k-apple-macos \
		  --prefix=$BUILD/gmp-m68k-INSTALL && \
    make && make install

# install mlton binaries (for bootstrapping custom mlton)
WORKDIR $BUILD
RUN wget https://github.com/MLton/mlton/releases/download/on-20210117-release/mlton-20210117-1.amd64-linux-glibc2.31.tgz && \
    tar xvzf mlton-20210117-1.amd64-linux-glibc2.31.tgz && \
    cd $BUILD/mlton-20210117-1.amd64-linux-glibc2.31 && make install

ADD https://api.github.com/repos/agoode/mlton/git/refs/heads/mac mlton-version.json
RUN git clone --depth 2 --branch mac https://github.com/agoode/mlton.git

# build mlton binary
# for boostrapping to work, we have to build at an earlier commit
RUN cd mlton && \
    git checkout mac^ && \
    export MLTON=$BUILD/mlton-INSTALL && \
    make all && \
    make PREFIX=$MLTON install

# build m68k runtime
RUN cd mlton && \
    git checkout mac && \
    git clean -dfx && \
    make \
		  TARGET_OS=macos \
		  TARGET_ARCH=m68k \
		  TARGET=m68k-apple-macos \
		  WITH_GMP_DIR=$BUILD/gmp-m68k-INSTALL \
		  PREFIX=$BUILD/mlton-INSTALL \
		  dirs runtime install-runtime

# patch the lack of existence of svnversion, needed for twelf build
RUN echo '#!/bin/bash\ngit rev-parse HEAD' > /usr/bin/svnversion && \
    chmod +x /usr/bin/svnversion


ADD https://api.github.com/repos/agoode/twelf/git/refs/heads/main twelf-version.json
RUN git clone --branch mac --depth 1 https://github.com/agoode/twelf

# build twelf library
# RUN cd twelf && \
#     make mlton="$BUILD/mlton-INSTALL/bin/mlton \
#         -default-ann 'nonexhaustiveMatch ignore' \
#         -default-ann 'nonexhaustiveBind ignore'" \
#         wasi
