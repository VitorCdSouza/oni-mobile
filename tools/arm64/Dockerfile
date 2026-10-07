# Cross toolchain and user-mode emulator for building and replaying the sim port on arm64.
FROM debian:trixie-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
        g++-aarch64-linux-gnu gcc-aarch64-linux-gnu qemu-user make python3 \
    && rm -rf /var/lib/apt/lists/*
