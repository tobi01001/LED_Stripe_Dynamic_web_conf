The libraries here forked / copied from their original source but they are no longer linked.
Reason is that 
a.) For FastLED we use the ESP8266 DMA variant from https://github.com/coryking/FastLED - but this oneseems to be no longer maintained.
b.) WS2812FX was used in the beginning / as a base but I now moved completely to Fastled and merged that with  different effects to the WS2812FX library idea.

All the initial work was done by the creaters of the libraries and I only modified them to my personal needs. However, I keep the work on github available to everyone....

You should not need to modify anything in here on you own (unless you actually want to).

--------


This directory is intended for the project specific (private) libraries.
PlatformIO will compile them to static libraries and link to executable file.

The source code of each library should be placed in separate directory, like
"lib/private_lib/[here are source files]".

For example, see how can be organized `Foo` and `Bar` libraries:

|--lib
|  |--Bar
|  |  |--docs
|  |  |--examples
|  |  |--src
|  |     |- Bar.c
|  |     |- Bar.h
|  |--Foo
|  |  |- Foo.c
|  |  |- Foo.h
|  |- readme.txt --> THIS FILE
|- platformio.ini
|--src
   |- main.c

Then in `src/main.c` you should use:

#include <Foo.h>
#include <Bar.h>

// rest H/C/CPP code

PlatformIO will find your libraries automatically, configure preprocessor's
include paths and build them.

More information about PlatformIO Library Dependency Finder
- http://docs.platformio.org/page/librarymanager/ldf.html
