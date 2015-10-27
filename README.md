# CEF based PhantomJS scratch repository

Can we implement PhantomJS on top of CEF? It looks like it!

## Setup

Clone this repository into a CEF root folder. Then, add the cloned folder as a
sub directory in the top-level CMakeLists.txt of CEF, e.g.:

    ...
    add_subdirectory(libcef_dll)
    add_subdirectory(cefclient)
    add_subdirectory(cefsimple)
    # this is what you need to add
    add_subdirectory(phantomjs-cef)

Now you are ready to compile phantomjs-cef:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cd phantomjs-cef
    make
    ./Debug/phantomjs ../phantomjs-cef/examples/test.js

I tested it against the 64bit Linux build of CEF in branches 2454 and 2526. You
can download CEF from: https://cefbuilds.com/

Note that this is a playground currently to see what works and what not. Working
examples are available in the `examples/` subfolder.

## Async API

Chromium uses a multiprocess architecture for stability and performance reasons.
As such, some synchronous parts of the old PhantomJS API cannot be supported.
Most notably, you cannot evaluate JavaScript and directly receive the return
value.

For this reason, the code presented here differs in some ways from the old
PhantomJS API. It leverages JavaScript promises to make working with the async
code as simple as possible. Look at `examples/load_promise.js` to get a feeling
of how this can look like.

## X11 Dependency

Chromium, and thus CEF, depends on X11. Thus, even though we will use the
offscreen rendering features of CEF, we will still need X11. To workaround this
on servers, use Xvfb:

    xvfb-run --server-args="-screen 0, 1024x868x24" ./Debug/phantomjs ...
