FROM ubuntu:22.04
RUN apt-get update
RUN apt-get install openssh-server sudo jq curl -y && \
apt-get clean && \
rm -rf /var/lib/{apt,dpkg,cache,log}/
RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 m -o
WORKDIR /app
COPY --chown=m:sudo . /app
# replace for linux-compatible line endings
RUN sed -i 's/\r$//' /app/StartServer.sh
USER m
CMD ./StartServer.sh
