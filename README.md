# Open Telemetry Interchange Container Format  

<p align="center"><img src="https://raw.githubusercontent.com/talpa-io/otic/develop/doc/icon128.png" alt="otic_logo"></p>


[![Actions Status](https://github.com/talpa-io/otic/workflows/build/badge.svg)](https://github.com/talpa-io/otic/actions)  

## Description  
OTIC (Open Telemetry Interchange Container Format) is a portable, configurable, performance-oriented serialization library 
written in C99.  
It features:  
- A simple and easy-to-use API
- Support for multiple channels
- Support of every basic and compounded types
- Customizable flusher and seeker callbacks
- Low memory footprint  
- Portable API (Soon WIN32 support)

## Install:  
Due to portability reasons (MSB / LSB for example), it is recommended to build the library using CMake.
To build `libOtic` and the `otic command line tool`:  
```bash
mkdir -p build && cd build
```
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make 
make test
```
The command line tool should appear in the `bin` directory and a static and shared libotic library in the `lib` 
directory.  
To list all available options, the following can be done:  
```bash
cmake .. -LH
```
To install otic and libotic system-wide, do:
```bash
sudo make install
```  

To use the library API, simply include `otic.h`:
```c
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

## Example
```c
#include <stdio.h>
#include <otic.h>
#include <errorno.h>
#include <stdlib.h>
#include <string.h>

#include <otic/errHand.h> // otic_strError()

static uint8_t flusher(uint8_t* src, size_t size, void* file)
{
    return fwrite(src, 1, size, (FILE*)file) != 0;
}

static uint8_t fetcher(uint8_t* dest, size_t size, void* data)
{
    return fread(dest, 1, size, (FILE*)data) != 0;
}

static uint8_t flusher1(double ts, const char* sensorName, const char* sensorUnit, const oval_t* value, void* data)
{
#define FPRINTER(mod, val) fprintf((FILE*)data, "%lf\t%s\t%s\t"#mod"mod\n", ts, sensorName, sensorUnit, val)
    switch (otic_oval_getType(value))
    {
        case OTIC_TYPE_NULL:
            fprintf((FILE*)data, "%lf\t%s\t%s\n", ts, sensorName, sensorUnit);
            break;
        case OTIC_TYPE_INT_POS:
            FPRINTER("%lu", value->val.lval);
            break;
        case OTIC_TYPE_INT_NEG:
            FPRINTER("-%lu", value->val.lval);
            break;
        case OTIC_TYPE_DOUBLE:
            FPRINTER("%lf", value->val.dval);
            break;
        case OTIC_TYPE_STRING:
            FPRINTER("%s", value->val.sval.ptr);
            break;
        case OTIC_TYPE_TRUE:
            FPRINTER("%s", "[bool: true]");
            break;
        case OTIC_TYPE_FALSE:
            FPRINTER("%s", "[bool: false]");
            break;
        case OTIC_TYPE_ARRAY:
            FPRINTER("%s", "Array");
            break;
        default:
            return 0;
    }
    return 1;
#undef FPRINTER
}

int main(void)
{  
    FILE* fileOut = fopen("pack_demo.otic", "wb");
    if (!fileOut) {
        perror("fopen()");
        return EXIT_FAILURE;
    }
    otic_pack_t oticPack;
    if (!otic_pack_init(&oticPack, 0x00, flusher, fileOut))
        goto pack_fail;
    otic_pack_channel_t* channel1 = otic_pack_defineChannel(&oticPack, OTIC_CHANNEL_TYPE_SENSOR, 1, 0x00, 2048);
    if (!channel1)
        goto pack_fail;
    otic_pack_channel_inject_i(channel1, 1234.4, "sensor1", "sensorUnit1", 1232434);
    otic_pack_channel_inject_d(channel1, 1234.5, "sensor2", "sensorUnit2", 3.1417);
    otic_pack_channel_inject_bool(channel1, 1234.5, "sensor2", "sensorUnit2", 0);
    otic_pack_channel_inject_i_neg(channel1, 1234.5, "sensor1", "sensorUnit1", 54);
    otic_pack_channel_inject_s(channel1, 12323, "sensor1", "sensorUnit1", "Some string");
    otic_pack_channel_inject_s(channel1, 12323, "sensor3", "sensorUnit3", "Some other string");
    otic_pack_channel_inject_n(channel1, 12456, "sensor1", "sensorUnit1");

    otic_pack_close(&oticPack);
    fclose(fileOut);

    FILE* srcFile = fopen("pack_demo.otic", "rb"), *destFile = fopen("unpack_demo.tsv", "w");
    otic_unpack_t oticUnpack;
    if (!otic_unpack_init(&oticUnpack, fetcher, srcFile, seeker, srcFile))
        goto unpack_fail;
    oticUnpackChannel_t *channel = otic_unpack_defineChannel(&oticUnpack, 1, flusher2, destFile);
    if (!channel)
        goto unpack_fail;
    while (otic_unpack_parse(&oticUnpack));
    otic_unpack_close(&oticUnpack);

    return EXIT_SUCCESS;

pack_fail:
    fprintf("Otic Error: %s\n", otic_strError());
    return EXIT_FAILURE;
unpack_fail:
    fprintf("Otic Error: %s\n", otic_strError(oticUnpack.error))
    return EXIT_FAILURE;
}
```