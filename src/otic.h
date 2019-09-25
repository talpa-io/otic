#ifndef OTIC
#define OTIC

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// error codes returned by various functions
#define OTIC_NO_ERROR 0
#define OTIC_ERROR_STRING_TOO_LARGE -1
#define OTIC_ERROR_TIMESTAMP_DECREASED -2
#define OTIC_ERROR_COLUMN_NULL -3
#define OTIC_ERROR_WRITE_ERROR -4

#define OTIC_ERROR_EOF -5
#define OTIC_ERROR_FILE_CORRUPT -6
#define OTIC_ERROR_COULDNT_ALLOCATE -7


// possible result types of the reader
#define OTIC_TYPE_INT 1
#define OTIC_TYPE_DOUBLE 2
#define OTIC_TYPE_STRING 3
#define OTIC_TYPE_NULL 4

// available statistic

// for writers, and columns
#define OTIC_STAT_NUM_ROWS 0
#define OTIC_STAT_NUM_BYTES 1

// just for the writer
#define OTIC_STAT_NUM_BYTES_TS 2
#define OTIC_STAT_NUM_BYTES_VALUES 3
#define OTIC_STAT_NUM_TS_SHIFT 4
#define OTIC_STAT_NUM_BLOCKS 5

// ______________________________________________________________________________________
// writer interface
//
typedef struct otic_writer* otic_writer;
typedef struct otic_column* otic_column;

// open a writer which will write into the given filename
// the result is NULL on error
otic_writer otic_writer_open_filename(const char* filename);

// same, but disable zstd compression for the output file
otic_writer otic_writer_open_filename_uncompressed(const char* filename);

// lower level interface: open a writer that will call the callback whenever
// the buffer is full and needs writing to disk/network/whatever
// the result is NULL on error
typedef int (*otic_write_cb_tp)(char *data, size_t size, void *userdata);
otic_writer otic_writer_open(otic_write_cb_tp cb, void *userdata);
otic_writer otic_writer_open_uncompressed(otic_write_cb_tp cb, void *userdata);

// flush buffer of the writer (either by writing to disk, or by calling the callback)
// returns one of the error codes OTIC_ERROR* on error
int otic_writer_flush(otic_writer);

// close and free the writer, doing a flush beforehand if necessary
int otic_writer_close(otic_writer);

// get one of a number of statistics OTIC_STAT_*
long otic_writer_get_statistics(otic_writer, int);

// writing data

// register a column with a writer. this will return a column in result
// returns one of the error codes OTIC_ERROR_* on error
// this must only be called once per column, usually right after opening the
// writer (but more columns can be registered at any point)
int otic_register_column(otic_writer, char *name, otic_column* result);

// register a column and also store some metadata with the column
int otic_register_column_metadata(otic_writer, char *name, char *metadata, size_t size, otic_column* result);

size_t otic_column_get_index(otic_column);
otic_column otic_writer_get_column(otic_writer, size_t);

long otic_column_get_statistics(otic_column, int);

// write various kinds of value at a given time into a column.
// time is specified by the arguments epoch (a UNIX epoch) and nanoseconds.
// time must be strictly increasing.

// write a long value
int otic_write_long(otic_column, time_t epoch, long nanoseconds, long value);
// write a double value
int otic_write_double(otic_column, time_t epoch, long nanoseconds, double value);
// write a string value
int otic_write_string(otic_column, time_t epoch, long nanoseconds, char *name, size_t size);
// write a NULL value
int otic_write_null(otic_column, time_t epoch, long nanoseconds);

// write columns using the index (slightly less efficient!)
// write a long value
int otic_write_long_index(otic_writer, size_t, time_t epoch, long nanoseconds, long value);
// write a double value
int otic_write_double_index(otic_writer, size_t, time_t epoch, long nanoseconds, double value);
// write a string value
int otic_write_string_index(otic_writer, size_t, time_t epoch, long nanoseconds, char *name, size_t size);
// write a NULL value
int otic_write_null_index(otic_writer, size_t, time_t epoch, long nanoseconds);


// ______________________________________________________________________________________
// reader interface
//
typedef struct otic_reader* otic_reader;
typedef struct otic_result* otic_result;
typedef size_t (*otic_read_cb_tp)(void* userdata, char *data, size_t size);

// open a reader which will read from a file name
// the result is NULL on error
otic_reader otic_reader_open_filename(const char* filename);

// open a reader which will read from a given FILE*
// the result is NULL on error
otic_reader otic_reader_open(otic_read_cb_tp cb, void* userdata);


// close and free the reader
int otic_reader_close(otic_reader);

// read next entry in file
// the result is only valid until the next call to otic_reader_next
// returns NULL on error or end of file
otic_result otic_reader_next(otic_reader);

// after NULL was returned by otic_reader_next, you can use
// otic_reader_geterror to find out the more precise error code
// errors are sticky, once the reader is in an error state,
// otic_reader_next will keep returning NULL
int otic_reader_geterror(otic_reader);

// after EOF is reached it's possible to ask for the last timestamp, specified
// in the closing block
time_t otic_reader_closing_epoch(otic_reader);
long otic_reader_closing_nanoseconds(otic_reader);

long otic_reader_get_statistics(otic_reader);


// getters for otic_result

// returns one of OTIC_TYPE_INT, OTIC_TYPE_DOUBLE, OTIC_TYPE_STRING, OTIC_TYPE_NULL
int otic_result_get_type(otic_result);

time_t otic_result_get_epoch(otic_result);
long otic_result_get_nanoseconds(otic_result);

long otic_result_get_long(otic_result);
double otic_result_get_double(otic_result);
// returns pointer to string in result, and size directly
// the memory is owned by the result and only valid until the next call to
// otic_reader_next
size_t otic_result_get_string(otic_result, char** result);

// return name of the column and metadata for the column. The same comments as
// otic_result_get_string apply
size_t otic_result_get_colname(otic_result, char** result);
size_t otic_result_get_metadata(otic_result, char** result);

// make it possible to ignore all future results from a column
int otic_result_ignore_column_from_now_on(otic_result);


#endif // #ifndef OTIC
