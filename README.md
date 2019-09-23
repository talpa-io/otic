# Otic Format (Open Telemetry Interchange Container Format)

This package defines a small binary data format for efficiently writing
time series files to disk in a compressed form. The data format stores sensor
values of potentially many sensors into a compact format. Right now it consists
of a lightweight C library for writing such files and a Python component
that can both write and read these files. There is also a PHP extension to
read and write the format.

The file format saves space by not repeatedly storing repeated values, by
storing the time stamps in a space-efficient way and by also applying
`zstd` compression.

# Directory Structure

- `src/` contains the C source of the reader and writer
- `src/oticreference` contains the Python3 reference implementation
- `examples/` a few C example programs that use the library
- `phpsrc/otic` contains the source of the PHP extension to read and write
  the data format

# C-API Documentation

## Walkthrough of an example using the writer

The following shows a small example program that uses the writer to output
the machines total free memory and the current load average. Both values
are read 10 times per second. The example is leaving out proper error
handling to make it more readable. The complete code of the example can be
found in `examples/resources.c`

First we need to open a writer by giving the filename where the data
should be stored:

```c
    otic_writer w = otic_writer_open_filename("output.utsfmt");
```

Afterwards, we should set up the two data columns that we want to store data
into, one for the free memory and one for the load average:

```c
    otic_column mem, loadavg;
    otic_register_column(w, "mem_free", &mem);
    otic_register_column(w, "load_average", &loadavg);
```

That's all the setup we need to start writing data into the output file.

To actually store a value, we can use one of the following functions, depending
on the type of the value: `otic_write_long/double/string`. In addition to
the value itself, these functions take the column that we want to write the
value into, and the time that the measurement was taken at. Time is stored in
epoch seconds plus an offset of nanoseconds (the fields of the a `timespec`
struct on Linux, so that the result of `clock_gettime` can be used a source of
timing information).

For our example program, the calls to write values look like this:

```c
    struct timespec t;
    struct timespec sleeptime;

    // sleep for 0.1s
    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = 100000000L;

    long mem_free = 0;
    double load_averages[3];

    for (int i = 0; i < 100; i++) {
        clock_gettime(CLOCK_REALTIME, &t);
        mem_free = get_free_system_memory();
        getloadavg(load_averages, 3);
        otic_write_long(mem, t.tv_sec, t.tv_nsec, mem_free);
        otic_write_double(loadavg, t.tv_sec, t.tv_nsec, load_averages[0]);
        nanosleep(&sleeptime, NULL);
    }
```

At the end of the program execution we have to close the file:

```c
    otic_writer_close(w);
```

In some situations it's not practical to close the file at the end of the
program (for example in embedded scenarios where power loss can occur). To
still ensure that data ends up on disk, the `otic_writer_flush` function
can be called that flushes the various buffers. Note however that very
frequent flushing (without very few values written in between flushes)
greatly reduces the efficiency of the compression algorithm.

## Walkthrough of an example using the writer

Let's look at how we would read the data back out again. We first need to
open a reader:

```c
otic_reader r = otic_reader_open_filename("somefilename.otic");
```

Then we can iterate through the content of the file like this:
```c
    while (true) {
        otic_result res = otic_reader_next(r);
        if (!res) {
            break;
        }
        ...
    }
```

There are a number of accessor functions for the `otic_result` type. Most
importantly, we can find out the time stamp of the value using the following
two functions:

```c
    time_t epoch = otic_result_get_epoch(res);
    long nanoseconds = otic_result_get_nanoseconds(res);
```

and the name of the column like this:

```c
    char* name;
    size_t size = otic_result_get_colname(res, &name);
```

To find out, what the type of the value that we just read is, use:

```c
    int typ = otic_result_get_type(res);
```

This will return one of `OTIC_TYPE_INT`, `OTIC_TYPE_DOUBLE`, `OTIC_TYPE_NULL`
or `OTIC_TYPE_STRING`. Depending on what type the result has, we can get the
value as follows:

```c
    long value = otic_result_get_long(res);
    // or
    double value = otic_result_get_double(res);
    // or
    char *name;
    size_t size = otic_result_get_string(res, &name);
```

For `OTIC_TYPE_NULL` there is no value to retrieve.

After we are done reading, we can check whether an error occurred (e.g. if the
file ended prematurely) using:

```c
    int error = otic_reader_geterror(r);
```

To close the writer, call:

```c
    otic_reader_close(r);
```

## Library reference writer

In the following, a brief description for all the functions of the library is given.

### Opening and closing a writer


```c
otic_writer otic_writer_open_filename(const char* filename);
otic_writer otic_writer_open_filename_uncompressed(const char* filename);
```

Opens a writer which will write to the given `filename`. Returns `NULL` on
error. The `uncompressed` variant does not use `zstd` to compress the data.


```c
typedef int (*otic_write_cb_tp)(char *data, size_t size, void *userdata);
otic_writer otic_writer_open(otic_write_cb_tp cb, void *userdata);
otic_writer otic_writer_open_uncompressed(otic_write_cb_tp cb, void *userdata);
```

These functions represent a more low level interface to storing data. A writer
opened with this interface will call the given callback whenever a block of
data is ready for writing.

```c
int otic_writer_flush(otic_writer);
```

Flush buffer of the writer (either by writing to disk, or by calling the
callback). Returns one of the error codes `OTIC_ERROR*` on error.


```c
int otic_writer_close(otic_writer);
```

Close and free the writer, doing a flush beforehand if necessary.  Returns
one of the error codes `OTIC_ERROR*` on error.


### Writing data


```c
int otic_register_column(otic_writer, char *name, otic_column* result);
```

Before it's possible to write data, a column needs to be registered wit
the writer for every sensor. This is done with the
`otic_writer_register_column` function. It will return a column in
`result` or returns one of the error codes `OTIC_ERROR_*` on error. The
function must only be called once per column, usually after opening the
writer (but more columns can be registered at any later point).


```c
int otic_register_column_metadata(otic_writer, char *name, char *metadata, size_t size, otic_column* result);
```

Sometimes it is useful to store extra metadata for a column (for example the
unit, or a version number or similar). This can be done by registering the
column using the `otic_register_column_metadata` function, which behaves
like `otic_register_column` but takes a string `metadata` and its `size` as
additional arguments. The name of the column and the metadata can only be up to
64KiB in size.


```c
int otic_write_long(otic_column, time_t epoch, long nanoseconds, long value);
int otic_write_double(otic_column, time_t epoch, long nanoseconds, double value);
int otic_write_string(otic_column, time_t epoch, long nanoseconds, char *name, size_t size);
```

Write various kinds of values (`long`, `double` or `char*` at a given time
into a column. Time is specified by the arguments epoch (a UNIX epoch) and
nanoseconds. Time must never decrease from one call to these functions to
the next. The functions return an error code `OTIC_ERROR_*` if the write
failed. If a write failed, this value will not end up in the file. Further
writes to the file can still proceed.
