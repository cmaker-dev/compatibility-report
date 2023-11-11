FROM archlinux:latest

RUN mkdir /report
RUN mkdir /compat-report-generator

#Where the report goes when it's done
VOLUME /report

# Install cmake
RUN pacman -Syu --noconfirm && pacman -S cmake wget --noconfirm
RUN pacman -S base-devel --noconfirm
RUN pacman -S git --noconfirm

#Copying over cmaker sub module
COPY cmaker /cmaker
COPY . /compat-report-generator

WORKDIR /cmaker

# Install cmake-generator
RUN cmake -DCMAKE_BUILD_TYPE=Release . && make -j20 && make install


#Get index
WORKDIR /compat-report-generator
RUN wget https://github.com/cmaker-dev/index/releases/latest/download/index.json


# build and run compat report
RUN cmaker run
