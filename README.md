# CEF based PhantomJS scratch repository

Can we implement PhantomJS on top of CEF?

Checkout this folder into a download/install of CEF,
I tested it against the 64bit Linux build of branch
2454 from 2015-10-12, obtained from:

https://cefbuilds.com/

Then, add this folder as a sub directory in the top-level
CMakeLists.txt of the CEF build and compile it.

Note that this is a playground currently to see what works
and what not.