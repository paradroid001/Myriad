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