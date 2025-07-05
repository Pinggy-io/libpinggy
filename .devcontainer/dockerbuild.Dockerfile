# Use an official Ubuntu base image
FROM ubuntu:24.04

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    clang \
    cmake \
    make \
    bash \
    git \
    screen \
    golang-go \
    libssh-dev \
    libssl-dev \
    python3 \
    python3-pip \
    python3-setuptools \
    python3-dev \
    vim \
    gdb \
    ssh \
    valgrind \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Symlink python for convenience
RUN ln -s /usr/bin/python3 /usr/bin/python || true

RUN mkdir -p /Users/abhijit/pinggy

# Set default shell
SHELL ["/bin/bash", "-c"]

# Set working directory
WORKDIR /Users/abhijit

# Default command
CMD ["bash"]
