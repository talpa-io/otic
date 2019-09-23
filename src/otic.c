#include "otic.h"
#include "internal.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define RETURNONERROR(call) {int error = (call); if (error) return error;}
#define RETURNONERROR_READER(call) {int error = (call); if (error) { r->errorcode = error; return NULL;}}
#define READER_READVARUINT(res) {\
    if (r->position >= r->block_size) {\
        r->errorcode = OTIC_ERROR_FILE_CORRUPT;\
        return NULL;\
    }\
    (res) = r->uncompressed_buffer[r->position++];\
    if ((res) & 0x80) {\
        RETURNONERROR_READER(_read_varuint_slow_path((res) & 0x7f, r, &(res)));\
    }\
}

otic_writer _writer_open(void) {
    otic_writer result = malloc(sizeof(struct otic_writer));

    if (!result) {
        return NULL;
    }

    result->outfile = NULL;
    result->compressor = NULL;
    result->block_header = NULL;
    result->other_to_free = NULL;

    result->columnarray = malloc(8 * sizeof(otic_result));
    if (!result->columnarray) {
        _writer_dealloc(result);
        return NULL;
    }
    result->capcolumns = 8;
    result->numcolumns = 0;

    result->first_epoch = 0;
    result->first_nanoseconds = 0;
    result->seen_timestamp_in_block = false;
    result->last_epoch = 0;
    result->last_nanoseconds = 0;

    result->position = 0;


    result->header_written = false;
    result->block_started = false;

    result->block_header = malloc(BUFSIZE + BLOCK_HEADER_SIZE);
    result->buffer = result->block_header + BLOCK_HEADER_SIZE;

    result->compressed_buffer = NULL;

    return result;
}


otic_writer otic_writer_open_uncompressed(otic_write_cb_tp cb, void *userdata) {
    otic_writer result = _writer_open();
    if (!result) {
        return result;
    }
    result->write_callback = cb;
    result->userdata = userdata;
    return result;
}


otic_writer otic_writer_open(otic_write_cb_tp cb, void *userdata) {
    otic_writer result = otic_writer_open_uncompressed(cb, userdata);
    return _create_compressor(result);
}

otic_writer otic_writer_open_filename_uncompressed(const char* filename) {
    otic_writer result = _writer_open();
    if (!result) {
        return result;
    }

    FILE * outfile = fopen(filename, "wb");
    if (!outfile) {
        _writer_dealloc(result);
        return NULL;
    }
    result->outfile = outfile;
    return result;
}

otic_writer otic_writer_open_filename(const char* filename) {
    otic_writer result = otic_writer_open_filename_uncompressed(filename);
    return _create_compressor(result);
}

otic_writer _create_compressor(otic_writer result) {
    if (!result) {
        return result;
    }
    result->compressor = ZSTD_createCCtx();
    if (!result->compressor) {
        _writer_dealloc(result);
        return NULL;
    }
    result->other_to_free = result->block_header;

    result->block_header = malloc(ZSTD_compressBound(BUFSIZE) + BLOCK_HEADER_SIZE);
    if (!result->block_header) {
        _writer_dealloc(result);
        return NULL;
    }
    result->compressed_buffer = result->block_header + BLOCK_HEADER_SIZE;

    return result;
}

int otic_writer_flush(otic_writer w) {
    if (!w->header_written) {
        char header[HEADER_SIZE] = {'T', 'l', 'P', 'a', 0, FILEFORMAT_VERSION, 0};
        char features = 0;
        if (w->compressor) {
            features |= FEATURE_COMPRESSION;
        }
        header[6] = features;
        RETURNONERROR(_write_to_output(w, header, HEADER_SIZE));
        w->header_written = true;
    }
    int res = 0;
    if (!w->block_started) {
        return OTIC_NO_ERROR;
    }
    size_t size;
    if (w->compressor) {
        size = ZSTD_compressCCtx(w->compressor,
                                 (void*)w->compressed_buffer,
                                 ZSTD_compressBound(BUFSIZE),
                                 (void*)w->buffer,
                                 w->position, 18);
        if (ZSTD_isError(size)) {
            return size;
        }
    } else {
        size = w->position;
    }
    _end_block(w, size);
    res = _write_to_output(w, w->block_header, size+BLOCK_HEADER_SIZE);
    fflush(w->outfile);
    w->position = 0;
    w->block_started = false;
    return res;
}

int _write_to_output(otic_writer w, char* buffer, size_t length) {
    if (w->outfile) {
        size_t bytes_written = fwrite(buffer, 1, length, w->outfile);
        if (bytes_written != length) {
            return OTIC_ERROR_WRITE_ERROR;
        }
    } else {
        RETURNONERROR(w->write_callback(buffer, length, w->userdata));
    }
    return OTIC_NO_ERROR;
}

int otic_writer_close(otic_writer w) {
    int res = otic_writer_flush(w);
    char end[9];
    end[0] =  BLOCK_TYPE_END;
    _write_uint32(end + 1, w->last_epoch);
    _write_uint32(end + 5, w->last_nanoseconds);
    _write_to_output(w, end, 9);
    if (w->outfile) {
        fclose(w->outfile);
    }
    _writer_dealloc(w);
    return res;
}

void _writer_dealloc(otic_writer w) {
    if (w->compressor) {
        ZSTD_freeCCtx(w->compressor);
    }

    free(w->block_header);
    free(w->other_to_free);

    for (size_t i = 0; i < w->numcolumns; i++) {
        _column_dealloc(w->columnarray[i]);
    }

    free(w->columnarray);

    free(w);
}

int otic_register_column(otic_writer w, char *name, otic_column* resultptr) {
    // ignore double column names for now
    _ensure_block_started(w);
    otic_column result = malloc(sizeof(struct otic_column));
    result->writer = w;
    result->last_value_type = OTIC_TYPE_NONE_SEEN;

    result->last_string_buffer = NULL;
    size_t namelen = strlen(name);
    result->name = malloc(namelen);
    if (!result->name) {
        _column_dealloc(result);
        *resultptr = NULL;
        return OTIC_ERROR_COULDNT_ALLOCATE;
    }
    memcpy(result->name, name, namelen);

    int error = _write_column_id_assignment(w, name, namelen);
    if (error) {
        _column_dealloc(result);
        *resultptr = NULL;
        return error;
    }
    if (w->numcolumns == w->capcolumns) {
        // no space left, realloc
        w->columnarray = realloc(
            w->columnarray, 2 * w->capcolumns * sizeof(otic_column));
        w->capcolumns *= 2;
    }

    unsigned long columnindex = w->numcolumns++;
    w->columnarray[columnindex] = result;
    result->columnindex = columnindex;

    *resultptr = result;
    return OTIC_NO_ERROR;
}

void _column_dealloc(otic_column c) {
    free(c->last_string_buffer);
    free(c->name);
    free(c);
}

int otic_register_column_metadata(otic_writer w, char *name, char *metadata, size_t size, otic_column* result) {
    RETURNONERROR(otic_register_column(w, name, result));
    _write_byte(w, TYPE_META_ASSIGN);
    _write_varuint(w, result[0]->columnindex);
    return _write_string_size(w, metadata, size);
}

int _maybe_flush(otic_writer w) {
    // we check BUFSIZE only after writing a value
    // BUFSIZE is conservatively chosen so that we can write one value, and
    // then multiplied by two. we flush, if we are past the halfway mark here
    if (w->position > BUFSIZE/2) {
        return otic_writer_flush(w);
    }
    return OTIC_NO_ERROR;
}

int _write_string_size(otic_writer w, char* value, size_t size) {
    if (size >= STRING_SIZE_LIMIT) {
        return OTIC_ERROR_STRING_TOO_LARGE;
    }
    _write_varuint(w, (unsigned long)size);
    memcpy(w->buffer + w->position, value, size);
    w->position += size;
    return OTIC_NO_ERROR;
}


int encode_varint_unsigned(unsigned long value, char* target) {
    bool more = true;
    int numwritten = 0;
    while (more) {
        unsigned char towrite = value & 0x7f;
        value >>= 7;
        if (value == 0) {
            more = false;
        } else {
            towrite |= 0x80;
        }
        target[numwritten++] = (char)towrite;
    }
    return numwritten;
}


void _write_varuint(otic_writer w, unsigned long value) {
    w->position += encode_varint_unsigned(value, w->buffer + w->position);
}

void _write_byte(otic_writer w, uint8_t value) {
    w->buffer[w->position++] = value;
}


int _write_column_id_assignment(otic_writer w, char* name, size_t namelen) {
    _write_byte(w, TYPE_NAME_ASSIGN);
    return _write_string_size(w, name, namelen);
}


int _write_timestamp(otic_writer w, time_t epoch, long nanoseconds) {
    if (!w->seen_timestamp_in_block) {
        w->first_epoch = w->last_epoch = epoch;
        w->first_nanoseconds = w->last_nanoseconds = nanoseconds;
        w->seen_timestamp_in_block = true;
        return OTIC_NO_ERROR;
    }
    unsigned long diff = epoch - w->last_epoch;
    if (!diff) {
        diff = nanoseconds - w->last_nanoseconds;
        if (!diff) {
            return OTIC_NO_ERROR;
        }
        if (diff > nanoseconds) { // underflow occured
            return OTIC_ERROR_TIMESTAMP_DECREASED;
        }
        _write_byte(w, TYPE_SHIFT_TIMESTAMP_NS);
        _write_varuint(w, diff);
        w->last_nanoseconds = nanoseconds;

        return OTIC_NO_ERROR;
    }
    if (diff > epoch) { // underflow occured
        return OTIC_ERROR_TIMESTAMP_DECREASED;
    }
    _write_byte(w, TYPE_SHIFT_TIMESTAMP_EPOCH);
    _write_varuint(w, diff);
    _write_varuint(w, nanoseconds);

    w->last_epoch = epoch;
    w->last_nanoseconds = nanoseconds;
    return OTIC_NO_ERROR;
}

int otic_write_long(otic_column c, time_t epoch, long nanoseconds, long value) {
    if (!c) {
        return OTIC_ERROR_COLUMN_NULL;
    }

    otic_writer w = c->writer;
    _ensure_block_started(w);
    RETURNONERROR(_write_timestamp(w, epoch, nanoseconds));

    if (c->last_value_type == OTIC_TYPE_INT &&
            c->last_value.int_value == value) {
        _write_byte(w, TYPE_UNMODIFIED);
        _write_varuint(w, c->columnindex);
    } else {
        if (0 <= value && value <= MAX_DIRECT_INT) {
            _write_byte(w, value);
            _write_varuint(w, c->columnindex);
        }
        else if (value >= 0) {
            _write_byte(w, TYPE_POS_INT);
            _write_varuint(w, c->columnindex);
            _write_varuint(w, (unsigned long)value);
        } else  {
            _write_byte(w, TYPE_NEG_INT);
            _write_varuint(w, c->columnindex);
            _write_varuint(w, (unsigned long)~value);
        }
        c->last_value_type = OTIC_TYPE_INT;
        c->last_value.int_value = value;
    }
    return _maybe_flush(w);
}

int otic_write_double(otic_column c, time_t epoch, long nanoseconds, double value) {
    if (!c) {
        return OTIC_ERROR_COLUMN_NULL;
    }

    otic_writer w = c->writer;
    _ensure_block_started(w);
    RETURNONERROR(_write_timestamp(w, epoch, nanoseconds));

    if (c->last_value_type == OTIC_TYPE_DOUBLE &&
            c->last_value.double_value == value) {
        _write_byte(w, TYPE_UNMODIFIED);
        _write_varuint(w, c->columnindex);
    } else {

        // reinterpret cast to unsigned int
        union {
            unsigned long lvalue;
            double dvalue;
        } pun;

        pun.dvalue = value;
        unsigned long lvalue = pun.lvalue;

        _write_byte(w, TYPE_DOUBLE);
        _write_varuint(w, c->columnindex);
        unsigned long mask = 0xff;
        mask <<= (7 * 8);
        for (int i = 0; i < 8; i ++) {
            _write_byte(w, (lvalue & mask) >> (8 * (7 - i)));
            mask >>= 8;
        }

        c->last_value_type = OTIC_TYPE_DOUBLE;
        c->last_value.double_value = value;
    }
    return _maybe_flush(w);
}

int otic_write_string(otic_column c, time_t epoch, long nanoseconds, char* value, size_t size) {
    if (!c) {
        return OTIC_ERROR_COLUMN_NULL;
    }

    if (size > STRING_SIZE_LIMIT) {
        return OTIC_ERROR_STRING_TOO_LARGE;
    }

    otic_writer w = c->writer;
    _ensure_block_started(w);
    RETURNONERROR(_write_timestamp(w, epoch, nanoseconds));

    if (c->last_value_type == OTIC_TYPE_STRING &&
            c->last_value.string_size == size &&
            memcmp(c->last_string_buffer, value, size) == 0) {
        _write_byte(w, TYPE_UNMODIFIED);
        _write_varuint(w, c->columnindex);
    } else {
        _write_byte(w, TYPE_STRING);
        _write_varuint(w, c->columnindex);
        _write_string_size(w, value, size);

        if (!c->last_string_buffer) {
            c->last_string_buffer = malloc(STRING_SIZE_LIMIT);
        }
        if (c->last_string_buffer) {
            // if malloc failed, too bad, we can ignore that problem
            memcpy(c->last_string_buffer, value, size);
            c->last_value_type = OTIC_TYPE_STRING;
            c->last_value.string_size = size;
        }
    }
    return _maybe_flush(w);
}

int otic_write_null(otic_column c, time_t epoch, long nanoseconds) {
    if (!c) {
        return OTIC_ERROR_COLUMN_NULL;
    }

    otic_writer w = c->writer;
    _ensure_block_started(w);
    RETURNONERROR(_write_timestamp(w, epoch, nanoseconds));
    _write_byte(w, TYPE_NULL);
    _write_varuint(w, c->columnindex);
    c->last_value_type = OTIC_TYPE_NULL;
    return OTIC_NO_ERROR;
}


void _ensure_block_started(otic_writer w) {
    if (w->block_started) {
        return;
    }
    w->block_header[0] = BLOCK_TYPE_DATA;
    w->block_started = true;
}

void _end_block(otic_writer w, size_t size) {
    char* buffer = w->block_header + 1;
    _write_uint32(buffer, size);
    buffer += 4;
    _write_uint32(buffer, w->first_epoch);
    buffer += 4;
    _write_uint32(buffer, w->first_nanoseconds);
    w->seen_timestamp_in_block = false;
}

void _write_uint32(char* buffer, uint32_t size) {
    uint32_t mask = 0xff << (3 * 8);
    for (int i = 0; i < 4; i++) {
        buffer[i] = (size & mask) >> (8 * (3 - i));
        mask >>= 8;
    }
}


size_t otic_column_get_index(otic_column c) {
    return c->columnindex;
}

otic_column _get_column(otic_writer w, size_t index) {
    return w->columnarray[index];
}

int otic_write_long_index(otic_writer w, size_t columnindex, time_t epoch, long nanoseconds, long value) {
    return otic_write_long(_get_column(w, columnindex), epoch, nanoseconds, value);
}

int otic_write_double_index(otic_writer w, size_t columnindex, time_t epoch, long nanoseconds, double value) {
    return otic_write_double(_get_column(w, columnindex), epoch, nanoseconds, value);
}
int otic_write_string_index(otic_writer w, size_t columnindex, time_t epoch, long nanoseconds, char *name, size_t size) {
    return otic_write_string(_get_column(w, columnindex), epoch, nanoseconds, name, size);
}
int otic_write_null_index(otic_writer w, size_t columnindex, time_t epoch, long nanoseconds) {
    return otic_write_null(_get_column(w, columnindex), epoch, nanoseconds);
}

// ______________________________________________________________________________________
// reader
//


otic_reader otic_reader_open_filename(const char* filename) {
    otic_reader result = malloc(sizeof(struct otic_reader));
    result->infile = NULL;
    result->errorcode = OTIC_NO_ERROR;
    result->decompressor = NULL;
    result->columnarray = NULL;

    result->numcolumns = 0;
    result->position = 0;
    result->block_size = 0;
    result->version = 0;
    result->owns_infile = 0;

    result->buffer = malloc(ZSTD_compressBound(BUFSIZE));
    if (!result->buffer) {
        _reader_dealloc(result);
        return NULL;
    }

    result->columnarray = malloc(8 * sizeof(otic_result));
    if (!result->columnarray) {
        _reader_dealloc(result);
        return NULL;
    }
    result->capcolumns = 8;

    FILE * infile = fopen(filename, "rb");
    if (!infile) {
        _reader_dealloc(result);
        return NULL;
    }
    result->infile = infile;
    result->owns_infile = true;
    result->last_epoch = result->last_nanoseconds = 0;
    return result;
}

otic_reader otic_reader_open_file(FILE* infile) {
    if (!infile) {
        return NULL;
    }
    otic_reader result = malloc(sizeof(struct otic_reader));
    result->infile = NULL;
    result->errorcode = OTIC_NO_ERROR;
    result->decompressor = NULL;
    result->columnarray = NULL;

    result->numcolumns = 0;
    result->position = 0;
    result->block_size = 0;
    result->version = 0;

    result->buffer = malloc(ZSTD_compressBound(BUFSIZE));
    if (!result->buffer) {
        _reader_dealloc(result);
        return NULL;
    }

    result->columnarray = malloc(8 * sizeof(otic_result));
    if (!result->columnarray) {
        _reader_dealloc(result);
        return NULL;
    }
    result->capcolumns = 8;

    result->infile = infile;
    result->owns_infile = false;
    result->last_epoch = result->last_nanoseconds = 0;
    return result;
}

// close and free the reader
int otic_reader_close(otic_reader r) {
    if (r->owns_infile) {
        fclose(r->infile);
    }
    _reader_dealloc(r);
    return OTIC_NO_ERROR;
}

void _reader_dealloc(otic_reader r) {
    free(r->buffer);
    for (size_t i = 0; i < r->numcolumns; i++) {
        _result_dealloc(r->columnarray[i]);
    }
    free(r->columnarray);
    if (r->decompressor) {
        free(r->uncompressed_buffer);
        ZSTD_freeDCtx(r->decompressor);
    }
    free(r);
}

void _result_dealloc(otic_result res) {
    free(res->metadata);
    free(res->last_string_buffer);
    free(res->name);
    free(res);
}


otic_result otic_reader_next(otic_reader r) {
    if (r->errorcode) {
        return NULL;
    }
    RETURNONERROR_READER(_ensure_header_read(r));
    while (true) {
        otic_result res = _otic_reader_next_every_column(r);
        if (!res) {
            return res;
        }
        if (!res->ignore_column) {
            return res;
        }
    }
}


otic_result _otic_reader_next_every_column(otic_reader r) {
    while (true) {
        uint8_t msgtype;
        unsigned long varuint;
        unsigned long column_id;
        // make sure that there is data to read in the current block left
        if (r->position >= r->block_size) {
            RETURNONERROR_READER(_read_block(r));
            if (r->block_size == 0) {
                return NULL; // EOF reached
            }
        }
        // we can read one byte safely due to the check abover
        msgtype = r->uncompressed_buffer[r->position++];
        if (msgtype == TYPE_SHIFT_TIMESTAMP_EPOCH) {
            READER_READVARUINT(varuint);
            r->last_epoch += varuint;
            READER_READVARUINT(varuint);
            r->last_nanoseconds = varuint;
            continue;
        } else if (msgtype == TYPE_SHIFT_TIMESTAMP_NS) {
            READER_READVARUINT(varuint);
            r->last_nanoseconds += varuint;
            continue;
        } else if (msgtype == TYPE_NAME_ASSIGN) {
            _add_new_column(r);
            continue;
        } else if (msgtype == TYPE_META_ASSIGN) {
            READER_READVARUINT(column_id);
            if (column_id >= r->numcolumns) {
                r->errorcode = OTIC_ERROR_FILE_CORRUPT;
                return NULL;
            }
            otic_result column = r->columnarray[column_id];
            RETURNONERROR_READER(_read_string_copy(r, &column->metadata, &column->lenmetadata));
            continue;
        }
        READER_READVARUINT(column_id);
        if (column_id >= r->numcolumns) {
            r->errorcode = OTIC_ERROR_FILE_CORRUPT;
            return NULL;
        }
        otic_result column = r->columnarray[column_id];
        if (msgtype <= MAX_DIRECT_INT) {
            return _return_value_long(column, msgtype);
        } else if (msgtype == TYPE_UNMODIFIED) {
            return column;
        } else if (msgtype == TYPE_POS_INT) {
            READER_READVARUINT(varuint);
            return _return_value_long(column, (long)varuint);
        } else if (msgtype == TYPE_NEG_INT) {
            READER_READVARUINT(varuint);
            return _return_value_long(column, ~(long)varuint);
        } else if (msgtype == TYPE_DOUBLE) {
            if (r->position + 7 >= r->block_size) {
                r->errorcode = OTIC_ERROR_FILE_CORRUPT;
                return NULL;
            }
            if (column->ignore_column) {
                r->position += 8;
                return column; // value not updated
            }
            // reinterpret cast from unsigned long
            uint64_t result = 0;
            for (int i = 0; i < 8; i++) {
                uint8_t byte = r->uncompressed_buffer[r->position++];
                result = (result << 8) | byte;
            }
            union {
                uint64_t lvalue;
                double dvalue;
            } pun;

            pun.lvalue = result;
            return _return_value_double(column, pun.dvalue);

        } else if (msgtype == TYPE_STRING) {
            size_t size;
            if (column->ignore_column) {
                // efficiently skip string
                READER_READVARUINT(size);
                if ((size > STRING_SIZE_LIMIT) ||
                        (r->position + size > r->block_size)) {
                    r->errorcode = OTIC_ERROR_FILE_CORRUPT;
                    return NULL;
                }
                r->position += size;
                return column; // value not updated
            } else {
                char* value;
                RETURNONERROR_READER(_read_string_copy(r, &value, &size));
                return _return_value_string(column, value, size);
            }
        } else if (msgtype == TYPE_NULL) {
            return _return_value_null(column);
        }
        r->errorcode = OTIC_ERROR_FILE_CORRUPT;
        return NULL;
    }
}

int _read_byte(otic_reader r, uint8_t* result) {
    if (r->position >= r->block_size) {
        return OTIC_ERROR_FILE_CORRUPT;
    }
    *result = r->uncompressed_buffer[r->position++];
    return OTIC_NO_ERROR;
}

int otic_reader_geterror(otic_reader r) {
    return r->errorcode;
}

int _add_new_column(otic_reader r) {
    char* name;
    size_t lenname;
    RETURNONERROR(_read_string_copy(r, &name, &lenname));
    otic_result result = malloc(sizeof(struct otic_result));
    if (!result) {
        return OTIC_ERROR_COULDNT_ALLOCATE;
    }
    result->reader = r;
    result->metadata = NULL;
    result->lenmetadata = 0;
    result->last_string_buffer = NULL;
    result->last_value_type = OTIC_TYPE_NONE_SEEN;
    result->name = name;
    result->lenname = lenname;
    result->columnindex = r->numcolumns;
    result->ignore_column = false;
    if (r->numcolumns == r->capcolumns) {
        // no space left, realloc
        r->columnarray = realloc(
            r->columnarray, 2 * r->capcolumns * sizeof(otic_result));
        r->capcolumns *= 2;
    }
    r->columnarray[r->numcolumns++] = result;
    return OTIC_NO_ERROR;
}

int _read_string_copy(otic_reader r, char** result, size_t* length) {
    unsigned long size;
    RETURNONERROR(_read_varuint(r, &size));
    if (size > STRING_SIZE_LIMIT) {
        return OTIC_ERROR_FILE_CORRUPT;
    }
    if (r->position + size <= r->block_size) {
        char* s = malloc(size + 1);
        if (!s) {
            return OTIC_ERROR_COULDNT_ALLOCATE;
        }
        memcpy(s, r->uncompressed_buffer + r->position, size);
        s[size] = 0; // 0-terminate
        r->position += size;
        *result = s;
        *length = size;
        return OTIC_NO_ERROR;
    } else {
        return OTIC_ERROR_FILE_CORRUPT;
    }
}

int inline _read_varuint(otic_reader r, unsigned long* result) {
    // optimized for reading one-byte varuints, which are by far the most
    // common
    unsigned long res = 0;
    if (r->position >= r->block_size) {
        return OTIC_ERROR_FILE_CORRUPT;
    }
    res = r->uncompressed_buffer[r->position++];
    if (!(res & 0x80)) {
        *result = res;
        return OTIC_NO_ERROR;
    }

    return _read_varuint_slow_path(res & 0x7f, r, result);
}

int _read_varuint_slow_path(unsigned long res, otic_reader r, unsigned long* result) {
    int shift = 7;
    while (shift < 64) {
        uint8_t byte;
        RETURNONERROR(_read_byte(r, &byte));
        res = res | ((byte & 0x7f) << shift);
        shift += 7;
        if (!(byte & 0x80)) {
            *result = res;
            return OTIC_NO_ERROR;
        }
    }
    return OTIC_ERROR_FILE_CORRUPT;
}


int _ensure_header_read(otic_reader r) {
    if (r->version) {
	return OTIC_NO_ERROR;
    }
    char header[HEADER_SIZE];
    int sizeread = fread(header, 1, HEADER_SIZE, r->infile);
    if (sizeread != HEADER_SIZE) {
	return OTIC_ERROR_EOF;
    }
    if (memcmp(header, "TlPa", 4) != 0) {
	return OTIC_ERROR_FILE_CORRUPT;
    }

    r->version = ((int)header[4] << 8) + (int)header[5];
    if (r->version != 1) {
        return OTIC_ERROR_FILE_CORRUPT;
    }

    int features = header[6];
    if (features & FEATURE_COMPRESSION) {
	r->decompressor = ZSTD_createDCtx();
	if (!r->decompressor) {
	    return OTIC_ERROR_COULDNT_ALLOCATE;
	}
        r->uncompressed_buffer = malloc(BUFSIZE);
	if (!r->uncompressed_buffer) {
	    return OTIC_ERROR_COULDNT_ALLOCATE;
	}
    } else {
        r->uncompressed_buffer = r->buffer;
    }
    return OTIC_NO_ERROR;
}

int _read_block(otic_reader r) {
    r->position = 0;

    char blockheader[1];
    int sizeread = fread(blockheader, 1, 1, r->infile);
    if (sizeread != 1) {
        return OTIC_ERROR_FILE_CORRUPT;
    }
    if (blockheader[0] == BLOCK_TYPE_DATA) {
        uint8_t header[12];
        sizeread = fread(header, 1, 12, r->infile);
        if (sizeread != 12) {
            return OTIC_ERROR_FILE_CORRUPT;
        }
        uint32_t isize = _read_uint32(header);
        if (isize > ZSTD_compressBound(BUFSIZE)) {
            return OTIC_ERROR_FILE_CORRUPT;
        }
        uint32_t epoch = _read_uint32(header + 4);
        r->last_epoch = epoch;
        uint32_t nanoseconds = _read_uint32(header + 8);
        r->last_nanoseconds = nanoseconds;
        sizeread = fread(r->buffer, 1, isize, r->infile);
        if (sizeread != isize) {
            return OTIC_ERROR_FILE_CORRUPT;
        }
        if (r->decompressor) {
            int sizedecompressed = ZSTD_decompressDCtx(
                    r->decompressor, r->uncompressed_buffer,
                    BUFSIZE, r->buffer, isize
            );
            if (ZSTD_isError(sizedecompressed)) {
                return OTIC_ERROR_FILE_CORRUPT;
            }
            r->block_size = sizedecompressed;
        } else {
            r->block_size = isize;
        }
        return OTIC_NO_ERROR;
    } else if (blockheader[0] == BLOCK_TYPE_END) {
        r->block_size = 0;
        uint8_t header[8];
        sizeread = fread(header, 1, 8, r->infile);
        if (sizeread != 8) {
            return OTIC_ERROR_FILE_CORRUPT;
        }
        uint32_t epoch = _read_uint32(header);
        r->last_epoch = epoch;
        uint32_t nanoseconds = _read_uint32(header + 4);
        r->last_nanoseconds = nanoseconds;
        return OTIC_NO_ERROR;
    }
    return OTIC_ERROR_FILE_CORRUPT;
}

uint32_t _read_uint32(uint8_t* buffer) {
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
        result = (result << 8) | buffer[i];
    }
    return result;
}

otic_result _return_value_long(otic_result column, long value) {
    column->last_value.int_value = value;
    column->last_value_type = OTIC_TYPE_INT;
    return column;
}

otic_result _return_value_double(otic_result column, double value) {
    column->last_value.double_value = value;
    column->last_value_type = OTIC_TYPE_DOUBLE;
    return column;
}

otic_result _return_value_string(otic_result column, char* value, size_t size) {
    // XXX could (re)use a buffer here instead
    free(column->last_string_buffer);
    column->last_value.string_size = size;
    column->last_string_buffer = value;
    column->last_value_type = OTIC_TYPE_STRING;
    return column;
}

otic_result _return_value_null(otic_result column) {
    column->last_value_type = OTIC_TYPE_NULL;
    return column;
}


time_t otic_reader_closing_epoch(otic_reader r) {
    if (r->block_size != 0) {
        // not at end
        return 0;
    }
    return r->last_epoch;
}

long otic_reader_closing_nanoseconds(otic_reader r) {
    if (r->block_size != 0) {
        // not at end
        return 0;
    }
    return r->last_nanoseconds;
}

int otic_result_get_type(otic_result res) {
    return res->last_value_type;
}

long otic_result_get_long(otic_result res) {
    return res->last_value.int_value;
}

double otic_result_get_double(otic_result res) {
    return res->last_value.double_value;
}

size_t otic_result_get_string(otic_result res, char** result) {
    *result = res->last_string_buffer;
    return res->last_value.string_size;
}

time_t otic_result_get_epoch(otic_result res) {
    return res->reader->last_epoch;
}

long otic_result_get_nanoseconds(otic_result res) {
    return res->reader->last_nanoseconds;
}

size_t otic_result_get_colname(otic_result res, char** result) {
    *result = res->name;
    return res->lenname;
}

size_t otic_result_get_metadata(otic_result res, char** result) {
    *result = res->metadata;
    return res->lenmetadata;
}


int otic_result_ignore_column_from_now_on(otic_result res) {
    res->ignore_column = true;
    return OTIC_NO_ERROR;
}
