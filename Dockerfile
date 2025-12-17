FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    default-jre \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/* \
    && pip install orderedmultidict

WORKDIR /app

COPY CMakeLists.txt Makefile /app/
COPY linter /app/linter
COPY external /app/external

RUN rm -rf build dbuild coverage-build && \
    make release


FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    default-jre \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/bin/lint /usr/local/bin/lint
