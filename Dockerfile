FROM debian:bullseye

RUN sed -i 's/deb.debian.org/linux.xidian.edu.cn\/mirrors/g' /etc/apt/sources.list
RUN apt update && apt upgrade && apt install -y build-essential make cmake cmake-extras zlib1g-dev libgmp-dev qtbase5-dev  --fix-missing

WORKDIR /opt

COPY . .
RUN mkdir -p build && cd build && cmake .. && make && cd ..

CMD ["/opt/build/bin/MeowComputingCenter"]

