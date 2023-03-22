FROM ubuntu:18.04 

LABEL maintainer="hyunjungg"

ARG DEBIAN_FRONTEND=noninteractive

RUN \  
    apt-get update && apt-get -y upgrade &&\
    apt-get install -y apache2 &&\
    apt-get install -y libapache2-mod-php

EXPOSE 80

CMD ["apachectl", "-D", "FOREGROUND"]