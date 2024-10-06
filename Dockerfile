FROM gcc:latest

ARG BOOST_VERSION=1.84.0

RUN apt-get update --fix-missing && apt-get install -y cmake nlohmann-json3-dev libfmt-dev libspdlog-dev

RUN cd /tmp && \
    BOOST_VERSION_MOD=$(echo $BOOST_VERSION | tr . _) && \
    wget https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    tar --bzip2 -xf boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    cd boost_${BOOST_VERSION_MOD} && \
    ./bootstrap.sh --prefix=/usr/local && \
    ./b2 install && \
    rm -rf /tmp/*

WORKDIR /app

COPY . .

RUN mkdir -p build

WORKDIR /app/build

RUN cmake ..

RUN make -j$(nproc)

CMD ["./blok_box_assignment"]
