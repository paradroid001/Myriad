# Myriad
Myriad Engine

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