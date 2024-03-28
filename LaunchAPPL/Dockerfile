FROM ghcr.io/autc04/retro68

RUN mkdir /root/work
WORKDIR /root/work
RUN git clone --depth 1 https://github.com/autc04/Retro68.git

RUN cp -r /root/work/Retro68/LaunchAPPL /root/app
COPY scripts/build.sh /root/build.sh
