#!/bin/bash

xvfb-run --server-args="-screen 0, 1024x868x24" /opt/phantomjs/phantomjs /opt/phantomjs/examples/java/javabindings.js $1
