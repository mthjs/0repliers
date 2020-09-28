FROM archlinux:latest

RUN pacman -Syu --noconfirm && pacman -Sy --noconfirm \
   clang \
   cmake \
   gcc \
   make \
   zeromq

WORKDIR /opt

CMD [ "make" ]
