# ----------------------------------------------------------------------------
# To run explicitly with make:
# docker run --rm -it \
#   --entrypoint bash \
#   -v /tmp/avr-hex:/workspace/build imageregistry.homenet.local/library/avr-devenv:latest
#
# docker image build -t imageregistry.homenet.local/library/avr-devenv:latest .
# docker push imageregistry.homenet.local/library/avr-devenv:latest
#
# Notes:
# - Set entrypoint to "make" to build directly.
# - The workspace will contain the inc-files for the AVR 328P and 2560P. To compile an asm-file with AVRA,
#   use avra FILENAME.asm or avra -I /usr/avr/include FILENAME.asm as needed.
#
# References:
# https://github.com/DarkSector/AVR/blob/master/asm/include/
# ----------------------------------------------------------------------------
# srv0001:/home/kelaro/code/avr-devenv

FROM ubuntu:latest

RUN apt update && apt install -y software-properties-common && add-apt-repository universe && apt update

RUN apt install -y \
    avra \
    gcc-avr avr-libc \
    make cmake \
    avrdude \
    gdb-avr \
    avarice \
    simavr \
    nano \
    git unzip \
    curl \
    python3 python3-pip python3-venv \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Arduino CLI
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh && \
    echo 'export PATH=$PATH:/workspace/bin' >> /etc/profile && \
    echo 'export PATH=$PATH:/workspace/bin' >> ~/.bashrc && \
    curl https://raw.githubusercontent.com/DarkSector/AVR/master/asm/include/m328Pdef.inc --output /workspace/m328Pdef.inc && \
    curl https://github.com/DarkSector/AVR/blob/master/asm/include/m2560def.inc --output /workspace/m2560def.inc

# set PATH and install Arduino core
ENV PATH="/workspace/bin:$PATH"
RUN arduino-cli core update && \
    arduino-cli core install arduino:avr

# Buildahhh.... Set up a venv for PlatformIO (since Ubuntu 22.04+ (and Debian 12+) enforce PEP 668, which prevents system-wide pip3 install modifications)
RUN python3 -m venv /opt/platformio && \
    /opt/platformio/bin/pip install platformio && \
    ln -s /opt/platformio/bin/platformio /usr/local/bin/platformio

ENTRYPOINT ["/bin/bash"]
