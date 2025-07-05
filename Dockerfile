FROM ubuntu:24.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Set up basic environment
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Install system dependencies and build tools
RUN apt-get update && apt-get install -y \
    # Core build tools
    build-essential \
    cmake \
    ninja-build \
    meson \
    pkg-config \
    git \
    curl \
    wget \
    unzip \
    python3 \
    python3-pip \
    python3-venv \
    # QEMU build dependencies
    libglib2.0-dev \
    libpixman-1-dev \
    libcap-ng-dev \
    libattr1-dev \
    libaio-dev \
    libbrlapi-dev \
    libbz2-dev \
    libcurl4-gnutls-dev \
    libgtk-3-dev \
    libiscsi-dev \
    libjpeg8-dev \
    libncurses5-dev \
    libnuma-dev \
    librbd-dev \
    librdmacm-dev \
    libsasl2-dev \
    libsdl2-dev \
    libseccomp-dev \
    libsnappy-dev \
    libssh-dev \
    libtool \
    libusb-1.0-0-dev \
    libusbredirparser-dev \
    libvde-dev \
    libvdeplug-dev \
    libvte-2.91-dev \
    libxen-dev \
    valgrind \
    zlib1g-dev \
    # ARM cross-compilation tools
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    gdb-multiarch \
    # ARM binary execution support
    qemu-user-static \
    binfmt-support \
    # Development tools
    vim \
    nano \
    gdb \
    tree \
    file \
    strace \
    ltrace \
    hexdump \
    # Documentation tools
    doxygen \
    graphviz \
    && rm -rf /var/lib/apt/lists/*

# Set up Python environment for QEMU scripts
RUN python3 -m pip install --upgrade pip setuptools wheel

# Create development user
RUN groupadd -g 1000 dev && \
    useradd -u 1000 -g dev -m -s /bin/bash dev && \
    echo "dev ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Create workspace directories
RUN mkdir -p /workspace/qemu-src \
             /workspace/qemu-build \
             /workspace/zephyr \
             /workspace/tests \
             /workspace/artifacts && \
    chown -R dev:dev /workspace

# Set up ARM binary execution
RUN update-binfmts --enable qemu-arm

# Set working directory
WORKDIR /workspace

# Switch to development user
USER dev

# Set up development environment
RUN echo 'export PATH="/workspace/qemu-build:$PATH"' >> ~/.bashrc && \
    echo 'export QEMU_BUILD_DIR="/workspace/qemu-build"' >> ~/.bashrc && \
    echo 'export ZEPHYR_BASE="/workspace/zephyr"' >> ~/.bashrc && \
    echo 'alias ll="ls -la"' >> ~/.bashrc && \
    echo 'alias qemu-debug="gdb-multiarch /workspace/qemu-build/qemu-system-arm"' >> ~/.bashrc

# Create basic project structure
RUN mkdir -p /workspace/s32k344-qemu/{src,tests,docs,scripts}

# Entry point for development
CMD ["/bin/bash"] 