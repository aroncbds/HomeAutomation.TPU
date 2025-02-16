FROM ubuntu:latest

RUN apt update && apt install -y \
    arduino-cli \
    gcc-avr avr-libc \
    make cmake \
    avrdude \
    gdb-avr \
    avarice \
    simavr \
    git unzip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

RUN arduino-cli core update && \
    arduino-cli core install arduino:avr

# Install recommended VSCode extensions
RUN apt install -y python3-pip && pip3 install platformio

ENTRYPOINT ["make"]
