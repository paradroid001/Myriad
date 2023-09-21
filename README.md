# Myriad
Myriad Engine

If you want debug tasks etc, copy Projects/vscode/.vscode into your .vscode

## Quick Top Level Build
`make PROJECT=Engine TARGET_OS=windows all`

`make PROJECT=Sample TARGET_OS=windows all`

You can also pass:  
TARGET_ARCH (default=x86_64)  
TARGET_OS (default=linux, options=windows,linux)  
RELEASE_PLATFORM (default=Desktop, options=Desktop,Web,Android)  
BUILD_MODE (default=Release, options=Debug,Release)  

zig is now used as the default compiler (`zig c++ -target $(TARGET_ARCH)-$(TARGET-OS)`)
If you really want to use g++, you can pass CXX=g++
This is useful for generating compile_commands.json via bear:
cd Sample
`bear -- make CXX=g++ all`


The below instructions are now out of date.  

### Linux Build

Build myriad first, then build the sample.  
Run from the correct directory, with the LD_LIBRARY_PATH set.

```
cd Engine
TARGET_PLATFORM=PLATFORM_DESKTOP make clean
TARGET_PLATFORM=PLATFORM_DESKTOP make all
cd ..
cd Sample
make all
cd build
LD_LIBRARY_PATH=. sample
```

### Windows Build

This works best with w64devkit, from https://github.com/skeeto/w64devkit  
Just clone the repo, build the docker image, and then unzip the devkit somewhere convenient:
```
docker build -t w64devkit .
docker run --rm w64devkit >w64devkit.zip
mv w64devkit.zip c:\compilers
cd c:\compilers
unzip w64devkit.zip
```
In the shell you will run `make` from, set your path:
`set PATH=c:\compilers\w64devkit\bin;%PATH%`

now you can make Myriad:
```
cd Engine
TARGET_PLATFORM=PLATFORM_DESKTOP make clean
TARGET_PLATFORM=PLATFORM_DESKTOP make all
cd ..
```
And now the sample, note we must rename to dll:
```
cd Sample
make all
cd build
mv libmyriad.so libmyriad.dll
sample.exe
```