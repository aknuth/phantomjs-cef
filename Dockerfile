FROM ubuntu
MAINTAINER Andreas Knuth

RUN apt-get update

RUN apt-get install -y libxcb1 xvfb libjpeg-turbo8 libicu52 libglib2.0-0 libfontconfig1 libpangocairo-1.0-0 libxi6 libxcomposite1 libnss3 libasound2 libxtst6 libgconf-2-4 libatk1.0-0 libxcursor1 libxss1 libxrandr2 libcups2 libpcre3 p7zip-full curl nano

ENV PHANTOMJS_HOME /opt/phantomjs

RUN curl -L -o phantomjs.7z https://doc-0g-5c-docs.googleusercontent.com/docs/securesc/ha0ro937gcuc7l7deffksulhg5h7mbp1/fk5650vegvv3pkqbr842uku7tdm5pbs9/1457193600000/08025283507272817459/*/0B627fGWpuqypVzVjN0tkR1pBRm8?e=download && \
	7z x phantomjs.7z && \
	mkdir ${PHANTOMJS_HOME} && \ 
	mv build/* ${PHANTOMJS_HOME}/

RUN adduser phantomjs --disabled-password --system && chown -R phantomjs:users ${PHANTOMJS_HOME}

RUN chmod a+x ${PHANTOMJS_HOME}/phantomjava.sh

WORKDIR /opt/phantomjs

EXPOSE 12345
EXPOSE 8887

USER phantomjs