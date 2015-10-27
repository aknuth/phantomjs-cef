# CEF based PhantomJS scratch repository

Can we implement PhantomJS on top of CEF?

Clone this repository into a download/install folder of CEF. Then, add this
folder as a sub directory in the top-level CMakeLists.txt of the CEF build
and compile it:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make
    ./Debug/phantomjs ../phantomjs-cef/examples/test.js

I tested it against the 64bit Linux build of CEF in branches 2454 and 2526. You
can download CEF from: https://cefbuilds.com/

Note that this is a playground currently to see what works and what not. Working
examples are available in the examples/ subfolder.

## X11 Dependency

Chromium, and thus CEF, depends on X11. Thus, even though we will use the
offscreen rendering features of CEF, we will still need X11. To workaround this
on servers, use Xvfb:

    xvfb-run --server-args="-screen 0, 1024x868x24" ./Debug/phantomjs ...
