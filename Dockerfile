FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Устаналиваем зависимости
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    default-jre \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем python зависимости
RUN pip3 install orderedmultidict

WORKDIR /app

# Копирование и сборка
COPY . .
RUN rm -rf build dbuild coverage-build
RUN make release

ENV PATH="/app/build/bin:${PATH}"

CMD ["lint", "--help"