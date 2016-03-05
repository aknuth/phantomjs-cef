# CEF based PhantomJS

PhantomJS is a headless Browser, scriptable with a JavaScript API. This PhantomJS-CEF Version aims to implement the original API but is based on the Chromium Embedded Framework and thus on the chromium browser.

## Setup

There are several ways to get phantomjs-cef ready to start:

1. Download a binary release. Actually there are binaries available for:
  - Ubuntu 14.04 & 15.10 (64bit)
  - CentOS 7 (64bit) should also work - but there's actually a problem with the libicu package version ...
  - Windows 7/8/10 (64bit)
  
2. Use a docker Container
 - Windows & MacOs User please install the docker toolbox first
 - docker run -t -i --rm aknuth/phantomcef

3. Build it on your own:

`a. Linux`

You need Qt5.4 at a minimum. If your system doesn't support this or a higher version, please build it first.

	git clone https://github.com/qtproject/qtbase.git
	git checkout 5.5.1 (probably in a closer future '5.6.0')
	./configure -opensource -confirm-license -no-pch -release -static -no-opengl -no-xcb -no-linuxfb -no-kms -no-eglfs -no-widgets -no-libinput -no-gstreamer -no-xkbcommon-evdev -no-glib -no-xcb-xlib -no-pulseaudio -no-alsa -no-gtkstyle -no-openssl -no-freetype -no-harfbuzz -nomake examples -gui -no-qpa-platform-guard -qpa minimal
	make -j2 # depends on the number of your cores
	export PATH=/pathToYourQt551/bin:$PATH

You might need to install a lot of dependencies, which vary from system to system ...

In addition the following tools are also needed (apt-get install):
 - ninja-build 
 - cmake

After that:

Clone this repository, then copy/link/move a CEF build folder into this folder.
The folder must be called `cef`. You can download CEF from https://cefbuilds.com/.

Once you have a build in the `cef` folder, you are ready to compile phantomjs-cef:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -GNinja ..
    ninja phantomjs
    # now run it:
    ./phantomjs-cef/phantomjs ../phantomjs-cef/examples/test.js

I tested it against the 64bit builds of CEF in branches 2526 and 2623 for Linux
and Windows.

Note that this is a playground currently to see what works and what not. Working
examples are available in the `examples/` subfolder.

`b. Windows`

On Windows, you'll need to download Visual Studio 2013 and then load its compiler
environment before compiling PhantomJS-CEF, i.e. before running the CMake command
specified above:

`VS2013:`

    call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64

`VS2015:`

	call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

Then, we need a static Qt installation of QtBase (location see above) configured with

    configure -static -static-runtime -opensource -confirm-license -make-tool jom -nomake examples -nomake tests -no-opengl

Add the `bin` folder containing `qmake` to your `PATH` environment
variable. Also, make sure `ninja` && `cmake` is in your `PATH`.
Either use a tool like `patheditor` or do it manually via:

    SET PATH=C:\ninja;c:\cmake;C:\path\to\your\Qt551\bin;%PATH%

Here are the links to download the dependencies:

* Microsoft Visual Studio 2013 Express for Desktop: https://www.microsoft.com/download/details.aspx?id=44914
* Ninja: https://github.com/martine/ninja/releases
* Qt: git clone git://code.qt.io/qt/qtbase.git
* CMake: https://cmake.org/download/

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

Actually Chromium, and thus CEF, depends on X11. Thus, even though we will use the
offscreen rendering features of CEF, we will still need X11. To workaround this
on Linux servers, use Xvfb:

    xvfb-run --server-args="-screen 0, 1024x868x24" ./Debug/phantomjs ...

## Message trash
There are a few output messages which can be slightly ignored:

`Linux`
 - [ERROR:browser_main_loop.cc(217)] Running without the SUID sandbox! See https://chromium.googlesource.com/chromium/src/+/master/docs/linux_suid_sandbox_development.md for more information on developing with the sandbox on.
 - Xlib:  extension "RANDR" missing on display ":99".
 - [ERROR:sandbox_linux.cc(334)] InitializeSandbox() called with multiple threads in process gpu-process
 - [ERROR:browser_gpu_channel_host_factory.cc(145)] Failed to create channel. 

`Windows`
 - SetProcessDpiAwareness(2) failed: COM error 0xffffffff80070005  (Unknown error 0x0ffffffff80070005), using 1
 - [ERROR:scoped_ole_initializer.cc(20)] Multiple OleInitialize() calls for thread 3284
 - [ERROR:ipc_channel_win.cc(511)] pipe error: 232
 - [ERROR:singleton_hwnd.cc(34)] Cannot create windows on non-UI thread!