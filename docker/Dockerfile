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

RUN groupadd -g 1000 myriad
RUN useradd -u 1000 -g myriad -ms /bin/bash myriad
USER myriad

WORKDIR /home/myriad
RUN wget https://ziglang.org/builds/zig-linux-x86_64-0.12.0-dev.464+a63a1c5cb.tar.xz
RUN tar xf zig-linux-x86_64-0.12.0-dev.464+a63a1c5cb.tar.xz
RUN ln -s zig-linux-x86_64-0.12.0-dev.464+a63a1c5cb zig
ENV PATH="/home/myriad/zig:$PATH"

WORKDIR /myriad
CMD ["tail","-f","/dev/forever"]


