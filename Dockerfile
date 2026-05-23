FROM docker.xuanyuan.me/library/ubuntu:22.04
RUN apt update && DEBIAN_FRONTEND=noninteractive apt install -y \
    g++-11 cmake make gdb && rm -rf /var/lib/apt/lists/*
ENV CXX=g++-20
WORKDIR /app
CMD ["bash", "-c", "mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ./cache_server"]