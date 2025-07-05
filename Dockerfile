FROM ubuntu:24.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Set up basic environment
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Update package lists first
RUN apt-get update

# Install core build tools first
RUN apt-get install -y \
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
    && rm -rf /var/lib/apt/lists/*

# Install QEMU build dependencies (basic set)
RUN apt-get update && apt-get install -y \
    libglib2.0-dev \
    libpixman-1-dev \
    libcap-ng-dev \
    libattr1-dev \
    libaio-dev \
    libbz2-dev \
    libcurl4-gnutls-dev \
    libjpeg8-dev \
    libncurses5-dev \
    libseccomp-dev \
    libsnappy-dev \
    libtool \
    libusb-1.0-0-dev \
    valgrind \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Install ARM cross-compilation tools
RUN apt-get update && apt-get install -y \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    gdb-multiarch \
    qemu-user-static \
    binfmt-support \
    && rm -rf /var/lib/apt/lists/*

# Install development and debugging tools
RUN apt-get update && apt-get install -y \
    vim \
    nano \
    gdb \
    tree \
    file \
    strace \
    doxygen \
    graphviz \
    && rm -rf /var/lib/apt/lists/*

# Try to install optional packages (may not be available on all systems)
RUN apt-get update && apt-get install -y \
    libnuma-dev \
    libsasl2-dev \
    libsdl2-dev \
    libssh-dev \
    || echo "Some optional packages not available, continuing..."

# Clean up
RUN rm -rf /var/lib/apt/lists/*

# Create non-root user for development
RUN if ! id -u 1000 >/dev/null 2>&1; then \
        useradd -m -s /bin/bash -u 1000 dev; \
    else \
        useradd -m -s /bin/bash dev; \
    fi && \
    usermod -aG sudo dev && \
    echo "dev ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set up ARM emulation
RUN update-binfmts --enable qemu-arm
RUN update-binfmts --enable qemu-aarch64

# Set working directory
WORKDIR /workspace

# Switch to dev user
USER dev

# Set up development environment
RUN echo 'alias ll="ls -la"' >> ~/.bashrc && \
    echo 'alias la="ls -la"' >> ~/.bashrc && \
    echo 'export PS1="\u@s32k344-dev:\w$ "' >> ~/.bashrc

# Default command
CMD ["/bin/bash"] 