FROM archlinux:latest

RUN mkdir /report

#Where the report goes when it's done
VOLUME /report

# Install cmake
RUN pacman -Syu --noconfirm && pacman -S cmake wget --noconfirm
RUN pacman -S base-devel --noconfirm
RUN pacman -S git --noconfirm

RUN git clone --recurse-submodules https://github.com/cmaker-dev/compatibility-report


# Install cmaker
WORKDIR /compatibility-report/cmaker
RUN cmake -DCMAKE_BUILD_TYPE=Release . && make -j24 && make install


WORKDIR /compatibility-report


#Get index
RUN wget https://github.com/cmaker-dev/index/releases/latest/download/index.json


# build and run compat report
RUN cmaker run
