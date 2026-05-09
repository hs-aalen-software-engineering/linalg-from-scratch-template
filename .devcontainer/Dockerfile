# Devcontainer for linalg-from-scratch
# Pre-installs Python (with uv) + a C++ toolchain (g++, cmake, libeigen3-dev)
# so students can focus on the matrix class, not on installation.

FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    g++ \
    gdb \
    cmake \
    ninja-build \
    libeigen3-dev \
    python3 \
    python3-pip \
    python3-venv \
    time \
    valgrind \
    ca-certificates \
    curl \
 && rm -rf /var/lib/apt/lists/*

# Install uv (https://github.com/astral-sh/uv) for the vscode user
USER vscode
RUN curl -LsSf https://astral.sh/uv/install.sh | sh
ENV PATH="/home/vscode/.local/bin:${PATH}"

WORKDIR /workspaces
