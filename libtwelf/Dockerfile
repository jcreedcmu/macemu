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
