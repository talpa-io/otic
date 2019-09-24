import os

from cffi import FFI

ffibuilder = FFI()

srcdir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

ffibuilder.cdef(
    """
typedef int... time_t;

#define OTIC_TYPE_INT ...
#define OTIC_TYPE_DOUBLE ...
#define OTIC_TYPE_STRING ...
#define OTIC_TYPE_NULL ...

#define OTIC_ERROR_FILE_CORRUPT ...
#define OTIC_ERROR_TIMESTAMP_DECREASED ...

typedef int (*otic_write_cb_tp)(char *data, size_t size, void *userdata);
typedef struct otic_writer* otic_writer;
typedef struct otic_column* otic_column;

extern "Python" int python_callback(char *data, size_t size, void *userdata);

int encode_varint_unsigned(unsigned long value, char* target);
otic_writer otic_writer_open(otic_write_cb_tp cb, void *userdata);
otic_writer otic_writer_open_filename(const char* filename);
otic_writer otic_writer_open_uncompressed(otic_write_cb_tp cb, void *userdata);
otic_writer otic_writer_open_filename_uncompressed(const char* filename);
int otic_writer_close(otic_writer);
int otic_writer_flush(otic_writer);

int otic_register_column(otic_writer, char *name, otic_column* result);
int otic_register_column_metadata(otic_writer, char *name, char *metadata, size_t size, otic_column* result);
int otic_write_long(otic_column, time_t epoch, long nanoseconds, long value);
int otic_write_double(otic_column, time_t epoch, long nanoseconds, double value);
int otic_write_string(otic_column, time_t epoch, long nanoseconds, char* value, size_t size);
int otic_write_null(otic_column, time_t epoch, long nanoseconds);

size_t otic_column_get_index(otic_column);
int otic_write_long_index(otic_writer, size_t, time_t epoch, long nanoseconds, long value);
int otic_write_double_index(otic_writer, size_t, time_t epoch, long nanoseconds, double value);
int otic_write_string_index(otic_writer, size_t, time_t epoch, long nanoseconds, char *name, size_t size);
int otic_write_null_index(otic_writer, size_t, time_t epoch, long nanoseconds);

// reader
typedef struct otic_reader* otic_reader;
typedef struct otic_result* otic_result;
typedef size_t (*otic_read_cb_tp)(void* userdata, char *data, size_t size);

extern "Python" size_t python_read_callback(void* userdata, char *data, size_t size);

otic_reader otic_reader_open_filename(const char* filename);
otic_reader otic_reader_open(otic_read_cb_tp cb, void* userdata);
int otic_reader_close(otic_reader);
otic_result otic_reader_next(otic_reader);
int otic_reader_geterror(otic_reader);
int otic_result_get_type(otic_result);
time_t otic_result_get_epoch(otic_result);
long otic_result_get_nanoseconds(otic_result);
long otic_result_get_long(otic_result);
double otic_result_get_double(otic_result);
size_t otic_result_get_string(otic_result, char** result);
size_t otic_result_get_colname(otic_result, char** result);
size_t otic_result_get_metadata(otic_result, char** result);
int otic_result_ignore_column_from_now_on(otic_result);

time_t otic_reader_closing_epoch(otic_reader);
long otic_reader_closing_nanoseconds(otic_reader);
"""
)

with open(os.path.join(srcdir, "otic.c")) as f:
    source = f.read()

ffibuilder.set_source("_internal", source, include_dirs=[srcdir], libraries=["zstd"])

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
