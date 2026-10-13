FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    libssl-dev \
    gcc-14 \
    g++-14 \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 60 --slave /usr/bin/g++ g++ /usr/bin/g++-14

RUN userdel -r ubuntu 2>/dev/null; groupdel ubuntu 2>/dev/null; groupadd -g 1000 builder && useradd -m -u 1000 -g 1000 builder

WORKDIR /build

