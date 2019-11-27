# Open Telemetry Interchange Container Format

[![Actions Status](https://github.com/talpa-io/otic/workflows/build/badge.svg)](https://github.com/talpa-io/otic/actions)  



## Binaries

All libraries are stored in `/dist` directory.




## Install:  
The installation requires CMake.  
Quickly run:  
```bash
sudo apt-get install cmake
```
on linus to install it. On other platforms, Google is your friend.  
To show the list of available options, do:  
```bash
mkdir build && cd build
cmake .. -LH
```  
The following lines for example will build otic without the second compression feature:  
```bash
mkdir build && cd build
cmake .. -DOTIC_PACK_NO_COMPRESSION=ON -DCMAKE_BUILD_TYPE=Release    
``` 
 
For basic installation, do:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
This would create an appropriate MakeFile, run it, and create a executable
inside the `bin` folder.  

Run:
```bash
cd build && sudo make install
```  
This command will build libotic shared and static Libraries into your system, create a single header `otic.h` file, place
 it into the include directory path of the system and create a command line tool `otic` for quick packing and unpacking files. 

To use the library, simply include `otic.h` like so:
```cpp
#include <otic.h>
```
and tell the Linker where to find the Linked Library like so for CMake:
```CMake
target_link_libraries(<NameOfYourTarget> otic)
```
or add to your compiler's flags
```bash
-lotic
```  
To use the command line tool, simply type `otic` in your terminal. A simple help should appear as so:  
```bash
Usage: otic [-p|-u|-c|-h|-v] [-i] inputFileName [-o] outputFileName
```  
Just in case your paths are not updated, do:  
```bash
sudo ldconfig
```
 
## Test
  - AFL + CMake:  
    Build: 
        ```bash
            cmake -DCMAKE_C_COMPILER=afl-gcc ..
        ```
