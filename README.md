# Open Telemetry Interchange Container Format

[![Actions Status](https://github.com/talpa-io/otic/workflows/build/badge.svg)](https://github.com/talpa-io/otic/actions)  

## Install:  
The installation requires CMake.  
Quickly run:  
```bash
sudo apt-get install cmake
```
on linus to install it. On other platforms, Google is your friend.  
  
Run:
```bash
mkdir build
cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release
make -C build
```
This would create an appropriate MakeFile, run it, and create a executable
inside the `bin` folder.  

Run:
```bash
cd build && sudo make install
```
to install a Shared Link Library inside in your system (`\usr\local\lib` on Linux) 
build a single header file combining the `.h` files of the `src` and saving into your system 
(usually into `\usr\local\include`).    

To use the library, simply include `otic.h` like so:
```cpp
#include <otic.h>
```
and tell the Linker where to find the Linked Library like so for CMake:
```CMake
target_link_libraries(<NameOfYourTarget> otic zstd)
```
or add to your compiler's flags
```bash
-llibotic
```  
Just in case your paths are not updated, do:  
```bash
sudo ldconfig
```
 
### Links:  
- https://facebook.github.io/zstd/zstd_manual.html  

## Test
  - AFL + CMake:  
    Build: 
        ```bash
            cmake -DCMAKE_C_COMPILER=afl-gcc ..
        ```
