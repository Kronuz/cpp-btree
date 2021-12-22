# Adapted from: https://git.io/JqIAN
FROM ubuntu:hirsute

ARG TINI_VERSION=v0.19.0

ENV LANG=C.UTF-8 LC_ALL=C.UTF-8
ENV DEBIAN_FRONTEND=noninteractive

# Install just enough for conda to work
RUN set -x && \
    apt-get update -y && \
    apt-get install -y --no-install-recommends \
    wget bzip2 ca-certificates apt-transport-https gnupg2 software-properties-common git && \
    (wget https://apt.llvm.org/llvm-snapshot.gpg.key -O - | apt-key add -) && \
    echo "deb https://apt.llvm.org/hirsute/ llvm-toolchain-hirsute-13 main" >> /etc/apt/sources.list && \
    echo "deb-src https://apt.llvm.org/hirsute/ llvm-toolchain-hirsute-13 main" >> /etc/apt/sources.list && \
    apt-get update -y && \
    apt-get install -y --no-install-recommends \
    cmake build-essential \
    libllvm13 llvm-13 llvm-13-dev llvm-13-runtime \
    clang-13 clang-tools-13 libclang-common-13-dev libclang-13-dev libclang1-13 \
    clang-format-13 clangd-13 \
    lld-13 libc++-13-dev libc++abi-13-dev libomp-13-dev libicu-dev && \
    apt-get remove -y apt-transport-https gnupg2 software-properties-common && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Keep $HOME clean (no .wget-hsts file), since HSTS isn't useful in this context
RUN set -x && \
    wget --no-hsts --quiet https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini -O /usr/local/bin/tini && \
    chmod +x /usr/local/bin/tini

RUN set -x && \
    ln -s /usr/bin/clang-13  /usr/local/bin/clang && \
    ln -s /usr/bin/clang++-13  /usr/local/bin/clang++ && \
    ln -s /usr/bin/clangd-13 /usr/local/bin/clangd && \
    ln -s /usr/bin/clang-format-13 /usr/local/bin/clang-format && \
    ln -s /usr/bin/clang-tidy-13 /usr/local/bin/clang-tidy

SHELL ["tini", "--", "/bin/bash", "-c"]
