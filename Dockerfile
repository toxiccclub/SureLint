FROM ubuntu:22.04

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

RUN rm -rf build dbuild coverage-build
RUN make release

ENV PATH="/app/build/bin:${PATH}"

CMD ["lint", "--help"]