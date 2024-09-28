# Use the latest Ubuntu image
FROM ubuntu:24.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Update the package list and install necessary packages
RUN apt-get update && \
    apt-get install -y \
    aspnetcore-runtime-8.0 \
    build-essential \
    clang \
    clang-format \
    cmake \
    cpputest \
    curl \
    default-jdk \
    doxygen \
    dotnet-runtime-8.0 \
    dotnet-sdk-8.0 \
    g++ \
    gcc \
    gcovr \
    gdb-multiarch \
    git \
    libclang-dev \
    llvm-dev \
    ninja-build \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Download and install arm-none-eabi-gcc for ARM Cortex-M cross-compilation
ENV ARM_GCC_URL=https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/download/v13.3.1-1.1/xpack-arm-none-eabi-gcc-13.3.1-1.1-linux-x64.tar.gz
RUN wget $ARM_GCC_URL -O /tmp/arm-none-eabi-gcc.tar.gz && \
    tar -xzf /tmp/arm-none-eabi-gcc.tar.gz -C /opt/ && \
    rm /tmp/arm-none-eabi-gcc.tar.gz && \
    ln -s /opt/xpack-arm-none-eabi-gcc-13.3.1-1.1/bin/* /usr/local/bin/

# Set the working directory
WORKDIR /workspace

# Default command
CMD ["/bin/bash"]
