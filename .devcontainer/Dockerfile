# ----------------------------------------------------------------------------
# To run explicitly with make:
# docker run --rm -it imageregistry.homenet.local/library/avr-devenv:latest make
# ..or set entrypoint to "make"
# ----------------------------------------------------------------------------
# srv0001:/home/kelaro/code/avr-devenv

FROM ubuntu:latest

RUN apt update && apt install -y software-properties-common && add-apt-repository universe && apt update

RUN apt install -y \
    gcc-avr avr-libc \
    make cmake \
    avrdude \
    gdb-avr \
    avarice \
    simavr \
    git unzip \
    curl \
    python3 python3-pip python3-venv \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Arduino CLI
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh && \
    echo 'export PATH=$PATH:/workspace/bin' >> /etc/profile && \
    echo 'export PATH=$PATH:/workspace/bin' >> ~/.bashrc

# set PATH and install Arduino core
ENV PATH="/workspace/bin:$PATH"
RUN arduino-cli core update && \
    arduino-cli core install arduino:avr

# Buildahhh.... Set up a venv for PlatformIO (since Ubuntu 22.04+ (and Debian 12+) enforce PEP 668, which prevents system-wide pip3 install modifications)
RUN python3 -m venv /opt/platformio && \
    /opt/platformio/bin/pip install platformio && \
    ln -s /opt/platformio/bin/platformio /usr/local/bin/platformio

ENTRYPOINT ["/bin/bash"]
