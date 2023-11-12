FROM archlinux:latest

#Where the report goes when it's done
RUN mkdir /report

VOLUME /report

COPY . /compatibility-report

# Install cmake
RUN pacman -Syu --noconfirm && pacman -S cmake wget --noconfirm
RUN pacman -S base-devel --noconfirm
RUN pacman -S git --noconfirm



# Install cmaker
WORKDIR /compatibility-report/cmaker
RUN cmake -DCMAKE_BUILD_TYPE=Release . && make -j24 && make install


WORKDIR /compatibility-report


#Get index
RUN wget https://github.com/cmaker-dev/index/releases/latest/download/index.json


# build and run compat report
ENTRYPOINT cmaker clean -c && cmaker run
