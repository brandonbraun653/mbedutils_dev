# Use the latest Ubuntu image
FROM ubuntu:latest

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Update the package list and install necessary packages
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    clang \
    clang-format \
    cmake \
    doxygen \
    g++ \
    gcc \
    gcovr \
    git \
    libclang-dev \
    llvm-dev \
    ninja-build \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Define the URL for arm-none-eabi-gcc
ENV ARM_GCC_URL=https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/download/v13.3.1-1.1/xpack-arm-none-eabi-gcc-13.3.1-1.1-linux-x64.tar.gz

# Download and install arm-none-eabi-gcc
RUN wget $ARM_GCC_URL -O /tmp/arm-none-eabi-gcc.tar.gz && \
    tar -xzf /tmp/arm-none-eabi-gcc.tar.gz -C /opt/ && \
    rm /tmp/arm-none-eabi-gcc.tar.gz && \
    ln -s /opt/xpack-arm-none-eabi-gcc-13.3.1-1.1/bin/* /usr/local/bin/

# Set the working directory
WORKDIR /workspace

# Clone the mbedutils_dev repository
RUN git clone https://github.com/brandonbraun653/mbedutils_dev.git /workspace/mbedutils_dev
# Checkout the main branch
RUN cd /workspace/mbedutils_dev && git checkout main
# Update all the submodules recursively
RUN cd /workspace/mbedutils_dev && git submodule update --init --recursive || true

# Default command
CMD ["/bin/bash"]