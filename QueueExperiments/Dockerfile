FROM ubuntu:bionic

# Install dependencies from repos
RUN apt update && \
    apt upgrade -y && \
		apt install -y \
		aptitude \
		autotools-dev \
		binutils-dev \
		bison \
		build-essential \
		ccache \
		flex \
		clang \
		git \
		libboost-all-dev \
		libbz2-dev \
		libdouble-conversion-dev \
		libevent-dev \
		libffi-dev \
		libgflags-dev \
		libgoogle-glog-dev \
		libgtest-dev \
		libiberty-dev \
		libicu-dev \
		libjemalloc-dev \
		liblz4-dev \
		liblzma-dev \
		libsnappy-dev \
		libssl-dev \
		libtbb-dev \
		libxml2-dev \
		libmpich-dev \
		make \
		pkg-config \
		python-dev \
		zlib1g-dev \
		wget


# Get correct version of CMAKE
RUN cd && \
    apt remove --purge --auto-remove cmake && \
    version=3.16 && \
    build=2 && \
    mkdir ~/temp && \
    cd ~/temp && \
    wget https://cmake.org/files/v$version/cmake-$version.$build.tar.gz && \
    tar -xzvf cmake-$version.$build.tar.gz && \
    cd cmake-$version.$build/ && \
    ./bootstrap && \
    make -j$(nproc) && \
    make install

# Install GASNet-Ex
RUN cd && \
    wget https://gasnet.lbl.gov/EX/GASNet-2020.3.0.tar.gz && \
    tar -xzf GASNet-2020.3.0.tar.gz && \
    cd GASNet-2020.3.0 && \
    mkdir build && \
    cd build && \
    ../configure --prefix=/home/lp616/.local --enable-ibv-multirail --with-ibv-max-hcas=2 --with-ibv-physmem-max='20 GB' && \
    make all -j$(nprocs)&& \
    make install && \
    make par

ADD lib utils *.cpp *.h CMakeList.txt /root/QueueExperiments/

RUN cd $HOME/QueueExperiments && \
    mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=RELEASE && \
    make -j$(nprocs)

