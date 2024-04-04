FROM debian:unstable AS base

ENV BUILD=/build

RUN mkdir $BUILD
WORKDIR $BUILD

# maybe some of these packages are not needed:
# ninja-build xz-utils

RUN apt-get update && \
    apt-get install -y git cmake clang ninja-build xz-utils libgmp-dev curl \
                       libmpfr-dev libmpc-dev libboost-all-dev bison flex texinfo ruby

#####


FROM base AS mlton-bootstrap

RUN apt-get install -y mlton

ADD https://github.com/agoode/mlton.git#mac mlton


#####


FROM base AS retro68

# Checkout Retro68
ADD --keep-git-dir https://github.com/agoode/Retro68.git#mlton Retro68
RUN cd Retro68 && git submodule update --init --recursive

# Build Retro68
RUN mkdir /opt/Retro68-INSTALL && \
    mkdir Retro68-build && \
    cd Retro68-build && \
    ../Retro68/build-toolchain.bash \
		--prefix=/opt/Retro68-INSTALL \
		--no-ppc \
		--no-carbon \
		--multiversal

ENV PATH="/opt/Retro68-INSTALL/bin:${PATH}"

#####


FROM retro68 AS gmp-m68k

# build gmp library for m68k target
ADD https://gmplib.org/download/gmp/gmp-6.3.0.tar.xz gmp-6.3.0.tar.xz
RUN tar xvf gmp-6.3.0.tar.xz
RUN cd $BUILD/gmp-6.3.0 && \
    ./configure --host=m68k-apple-macos \
                --prefix=/opt/gmp-m68k-INSTALL && \
    make && make install


#####


FROM mlton-bootstrap AS mlton-self

# build mlton binary
RUN cd mlton && \
    make all && \
    make PREFIX=/opt/mlton-INSTALL install


#####


FROM mlton-bootstrap AS mlton-m68k
COPY --link --from=gmp-m68k /opt/gmp-m68k-INSTALL /opt/gmp-m68k-INSTALL
COPY --link --from=retro68 /opt/Retro68-INSTALL /opt/Retro68-INSTALL
ENV PATH="/opt/Retro68-INSTALL/bin:${PATH}"

# build m68k runtime
RUN cd mlton && \
    make \
        CC=gcc \
        USE_PREGEN=true \
        TARGET_OS=macos \
	TARGET_ARCH=m68k \
	TARGET=m68k-apple-macos \
	WITH_GMP_DIR=/opt/gmp-m68k-INSTALL \
	PREFIX=/opt/mlton-INSTALL \
	dirs runtime install-runtime


#####


FROM retro68 AS twelf
COPY --link --from=mlton-self /opt/mlton-INSTALL /opt/mlton-INSTALL
COPY --link --from=mlton-m68k /opt/mlton-INSTALL /opt/mlton-INSTALL
COPY --link --from=gmp-m68k /opt/gmp-m68k-INSTALL /opt/gmp-m68k-INSTALL

# patch the lack of existence of svnversion, needed for twelf build
RUN echo '#!/bin/bash\ngit rev-parse HEAD' > /usr/bin/svnversion && \
    chmod +x /usr/bin/svnversion

# check out agoode's twelf branch
ADD https://github.com/agoode/twelf.git#mac twelf

# build twelf library
RUN cd twelf && \
    make mlton="/opt/mlton-INSTALL/bin/mlton \
        -default-ann 'nonexhaustiveMatch ignore' \
        -default-ann 'nonexhaustiveBind ignore' \
        -verbose 1" \
        classic-mac

# give more standard names
RUN mv /build/twelf/bin/twelf-server.a /build/twelf/bin/libtwelf.a && \
    mv /build/twelf/bin/twelf_server.h /build/twelf/bin/libtwelf.h

# install build script
COPY scripts/build.sh /root/build.sh
