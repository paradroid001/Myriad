FROM ubuntu:jammy-20260509

# Tool Versions
# ZIG: See https://ziglang.org/download/
ENV ZIG_VERSION=zig-x86_64-linux-0.17.0-dev.1397+4331ba0fb
# EMSDK: See https://github.com/emscripten-core/emsdk/tags
ENV EMSDK_VERSION=5.0.7
# Doxygen: See https://www.doxygen.nl/download.html
ENV DOXYGEN_VERSION=1.17.0


RUN apt-get update -yqq && \
    apt-get install -yq binutils \
    build-essential \
    gdb \
    make \
    catch2 \
    wget \
    curl \
    python3.10 \
    python3-pip \
    less \
    file \
    vim \
    git \
    valgrind \
    sudo \
    apt-transport-https \
    ca-certificates \
    gnupg \
    software-properties-common \
    zip \
    unzip

# Get the cmake that we need - at least 3.25
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
RUN echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

RUN apt-get update -yqq && \
    apt-get install -yq kitware-archive-keyring \
                        cmake \
                        mingw-w64

#libx11-dev .................   for X11/Xlib.h
#mesa-common-dev........        for GL/glx.h
#libglu1-mesa-dev .....         for GL/glu.h
#libxrandr-dev ...........      for X11/extensions/Xrandr.h
#libxi-dev ...................  for X11/extensions/XInput.h

# dev dependency packages
RUN apt-get update -yqq && \
    apt-get install -yq libx11-dev \
                        mesa-common-dev \
                        libglu1-mesa-dev \
                        xorg-dev

# libcapstone-dev - for Tracy server
RUN apt-get update -yqq && \
    apt-get install -yq libcapstone-dev \
                        libdbus-1-dev \
                        pkg-config \
                        libssl-dev \
                        libegl1-mesa-dev \
                        libwayland-dev \
                        libxkbcommon-dev \
                        libnghttp2-dev \
                        libidn2-dev \
                        libssh2-1-dev \
                        libglfw3-dev


RUN groupadd -g 1000 myriad
RUN useradd -u 1000 -g myriad -ms /bin/bash myriad
USER myriad
WORKDIR /home/myriad

### ZIG
RUN wget https://ziglang.org/builds/${ZIG_VERSION}.tar.xz
RUN tar xf ${ZIG_VERSION}.tar.xz
RUN ln -s ${ZIG_VERSION} zig
ENV PATH="/home/myriad/zig:$PATH"
#support for running zig under windows as if it is gcc
#these are then referenced in the windows toolchain file,
#TC-windows.cmake
COPY --chmod=755 zig_c_windows.sh /home/myriad
COPY --chmod=755 zig_cpp_windows.sh /home/myriad

### Emscripten
#RUN git clone https://github.com/emscripten-core/emsdk.git
RUN wget https://github.com/emscripten-core/emsdk/archive/refs/tags/${EMSDK_VERSION}.zip
RUN unzip ${EMSDK_VERSION}.zip
WORKDIR /home/myriad/emsdk-${EMSDK_VERSION}
RUN ./emsdk install latest
RUN ./emsdk activate latest
RUN . ./emsdk_env.sh
RUN ln -s /home/myriad/emsdk-${EMSDK_VERSION} /home/myriad/emsdk

### Doxygen
WORKDIR /home/myriad
RUN wget https://www.doxygen.nl/files/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz
RUN tar xf doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz
RUN ln -s doxygen-${DOXYGEN_VERSION} doxygen
ENV PATH="/home/myriad/doxygen/bin:$PATH"

CMD ["tail","-f","/dev/forever"]
