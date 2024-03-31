FROM jcreed/libtwelf

RUN mv /build/twelf/bin/twelf-server.a /build/twelf/bin/libtwelf.a && \
    mv /build/twelf/bin/twelf_server.h /build/twelf/bin/libtwelf.h

COPY scripts/build.sh /root/build.sh
