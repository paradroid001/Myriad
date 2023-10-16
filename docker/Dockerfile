FROM bitnami/minideb:bullseye

RUN apt-get update -yqq && \
    apt-get install -yq binutils \
    build-essential \
    make \
    wget \
    curl \
    python3 \
    python3-pip \
    bear \
    less \
    file \
    vim


#libx11-dev .................   for X11/Xlib.h
#mesa-common-dev........        for GL/glx.h
#libglu1-mesa-dev .....         for GL/glu.h
#libxrandr-dev ...........      for X11/extensions/Xrandr.h
#libxi-dev ...................  for X11/extensions/XInput.h

# dev dependency packages
RUN apt-get install -yq libx11-dev \
    mesa-common-dev \
    libglu1-mesa-dev \
    xorg-dev

RUN groupadd -g 1000 myriad
RUN useradd -u 1000 -g myriad -ms /bin/bash myriad
USER myriad

WORKDIR /home/myriad
RUN wget https://ziglang.org/builds/zig-linux-x86_64-0.12.0-dev.464+a63a1c5cb.tar.xz
RUN tar xf zig-linux-x86_64-0.12.0-dev.464+a63a1c5cb.tar.xz
RUN ln -s zig-linux-x86_64-0.12.0-dev.464+a63a1c5cb zig
ENV PATH="/home/myriad/zig:$PATH"

RUN wget https://www.doxygen.nl/files/doxygen-1.9.8.linux.bin.tar.gz
RUN tar xf doxygen-1.9.8.linux.bin.tar.gz
RUN ln -s doxygen-1.9.8 doxygen
ENV PATH="/home/myriad/doxygen/bin:$PATH"

WORKDIR /myriad
CMD ["tail","-f","/dev/forever"]


