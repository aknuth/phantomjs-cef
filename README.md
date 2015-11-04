# CEF based PhantomJS

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
    ...

Now you are ready to compile phantomjs-cef:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -GNinja ..
    ninja phantomjs
    # now run it:
    ./phantomjs-cef/phantomjs ../phantomjs-cef/examples/test.js

I tested it against the 64bit builds of CEF in branches 2454 and 2526 for Linux
and Windows. You can download CEF from: https://cefbuilds.com/

Note that this is a playground currently to see what works and what not. Working
examples are available in the `examples/` subfolder.

## Windows Dependencies

On Windows, you'll need to download Visual Studio 2013 and then load its compiler
environment before compiling PhantomJS-CEF, i.e. before running the CMake command
specified above:

    call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64

Then, make sure your Qt installation can be found by CMake. For this it should
be enough to add the `bin` folder containing `qmake` to your `PATH` environment
variable. Also, make sure `ninja` is in your `PATH`.

Either use a tool like `patheditor` or do it manually via:

    SET PATH=C:\ninja;C:\Qt\5.5\msvc2013_64\bin;%PATH%

Here are the links to download the dependencies:

* Microsoft Visual Studio 2013 Express for Desktop: https://www.microsoft.com/download/details.aspx?id=44914
* Ninja: https://github.com/martine/ninja/releases
* Qt: http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe

## Windows Static Build

The PhantomJS-CEF builds provided here on GitHub are build against a static Qt 5
and static MSVC runtime. Qt was configured with

    configure -static -static-runtime -nomake tests -nomake examples -opensource

Then build `qtbase` and once it is finished, add the generated `bin` folder which
contains `qmake` into your `PATH` environment variable (see above). Once done,
continue with PhantomJS-CEF as explained above, but additionally pass
`-DSTATIC_QT_BUILD=1` to CMake as shown below:

    cmake -DSTATIC_QT_BUILD=1 -DCMAKE_BUILD_TYPE=Release -GNinja ..

## Async API

Chromium uses a multiprocess architecture for stability and performance reasons.
As such, some synchronous parts of the old PhantomJS API cannot be supported.
Most notably, you cannot evaluate JavaScript and directly receive the return
value.

For this reason, the code presented here differs in some ways from the old
PhantomJS API. It leverages JavaScript promises to make working with the async
code as simple as possible. Look at `examples/load_promise.js` to get a feeling
of how this can look like.

## X11 Dependency on Linux

Chromium, and thus CEF, depends on X11. Thus, even though we will use the
offscreen rendering features of CEF, we will still need X11. To workaround this
on Linux servers, use Xvfb:

    xvfb-run --server-args="-screen 0, 1024x868x24" ./Debug/phantomjs ...
