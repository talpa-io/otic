from contextlib import contextmanager

import pytest
from hypothesis import example, given, strategies, assume
from oticreference._internal import ffi, lib
from oticreference.test.test_reference import (
    msgcmp,
    values_with_repetitions,
    _write_all,
    convert_float_ts,
)
from oticreference.reference import (
    Constants,
    FileReader,
    Reader,
    Writer,
    decode_varint_unsigned,
    encode_varint_unsigned,
)


class Data:
    pass


@contextmanager
def get_output(compression=False):
    d = Data()
    r = bytearray()
    h = ffi.new_handle(r)
    if compression:
        w = lib.otic_writer_open(lib.python_callback, h)
    else:
        w = lib.otic_writer_open_uncompressed(lib.python_callback, h)
    d.writer = w
    yield d
    if w:
        lib.otic_writer_close(w)
    d.result = bytes(r)


@ffi.def_extern()
def python_callback(data, size, userdata):
    userdata = ffi.from_handle(userdata)
    if not isinstance(userdata, bytearray):
        return 1
    userdata.extend(ffi.unpack(data, size))
    return 0


@example(2 ** 32)
@example(2 ** 64 - 1)
@example(2 ** 64 - 2)
@example(4294967294)
@given(strategies.integers(min_value=0, max_value=2 ** 64))
def test_varint_hypothesis(i):
    res = bytearray(100)
    w = lib.encode_varint_unsigned(i, ffi.from_buffer(res, require_writable=True))
    res, pos = decode_varint_unsigned(res)
    assert res == i
    assert pos == w


def test_open_close():
    with get_output() as d:
        pass  # just open and close
    assert d.result == b"TlPa\x00\x01\x00\x02" + b"\x00" * 8


def test_write_nothing(tmp_path):
    from oticreference.test.test_creader import check_results_fn

    p = tmp_path / "foo.fmt"
    with get_output() as d:
        res = ffi.new("otic_column*")
        error = lib.otic_register_column(d.writer, b"test", res)
        assert not error
    msgcmp(
        [d.result],
        b"TlPa\x00\x01\x00",
        Constants.BLOCK_TYPE_DATA,
        0,
        0,
        0,
        6,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,  # initial time shift
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"test",
        Constants.BLOCK_TYPE_END,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    )

    with open(p, "wb") as f:
        f.write(d.result)

    # regression test: the above file should not break the readers!
    r = FileReader(p)
    with pytest.raises(StopIteration):
        r.read()
    check_results_fn(bytes(p), [])


def test_write_one():
    with get_output() as d:
        res = ffi.new("otic_column*")
        lib.otic_register_column(d.writer, b"test", res)
        lib.otic_write_long(res[0], 1, 0, 2)
    msgcmp(
        [d.result],
        b"TlPa\x00\x01\x00",
        Constants.BLOCK_TYPE_DATA,
        0,
        0,
        0,
        8,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,  # initial time shift
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"test",
        2,
        0,
        Constants.BLOCK_TYPE_END,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
    )


def test_flush_is_idempotent():
    with get_output() as d:
        res = ffi.new("otic_column*")
        lib.otic_register_column(d.writer, b"test", res)
        lib.otic_write_long(res[0], 1, 0, 2)
        lib.otic_writer_flush(d.writer)
        lib.otic_writer_flush(d.writer)
        lib.otic_writer_flush(d.writer)
        lib.otic_writer_flush(d.writer)
        lib.otic_writer_flush(d.writer)
        lib.otic_writer_flush(d.writer)
        lib.otic_writer_flush(d.writer)
    msgcmp(
        [d.result],
        b"TlPa\x00\x01\x00",
        Constants.BLOCK_TYPE_DATA,
        0,
        0,
        0,
        8,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,  # initial time shift
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"test",
        2,
        0,
        Constants.BLOCK_TYPE_END,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
    )


def test_write():
    with get_output() as d:
        res = ffi.new("otic_column*")
        error = lib.otic_register_column(d.writer, b"test", res)
        assert not error
        column = res[0]
        lib.otic_write_long(column, 1, 0, 2)
        lib.otic_write_long(column, 1, 0, 2)
        lib.otic_write_long(column, 1, 1, 2)
        lib.otic_write_long(column, 1, 1, 1000)
        lib.otic_write_long(column, 1, 1, -5000)
    msgcmp(
        [d.result],
        b"TlPa\x00\x01\x00",
        Constants.BLOCK_TYPE_DATA,
        0,
        0,
        0,
        22,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"test",
        2,
        0,
        Constants.TYPE_UNMODIFIED,
        0,
        Constants.TYPE_SHIFT_TIMESTAMP_NS,
        1,
        Constants.TYPE_UNMODIFIED,
        0,
        Constants.TYPE_POS_INT,
        0,
        encode_varint_unsigned(1000),
        Constants.TYPE_NEG_INT,
        0,
        encode_varint_unsigned(~-5000),
        Constants.BLOCK_TYPE_END,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        1,
    )


def test_write_double():
    with get_output() as d:
        res = ffi.new("otic_column*")
        error = lib.otic_register_column(d.writer, b"test", res)
        assert not error
        column = res[0]
        lib.otic_write_double(column, 1, 0, 2.5)
        lib.otic_write_double(column, 1, 0, 2.5)
        lib.otic_write_double(column, 1, 1, 2.5)
        lib.otic_write_double(column, 1, 1, 1000.34)
        lib.otic_write_double(column, 1, 1, -5000.54)
    msgcmp(
        [d.result],
        b"TlPa\x00\x01\x00",
        Constants.BLOCK_TYPE_DATA,
        0,
        0,
        0,
        42,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"test",
        Constants.TYPE_DOUBLE,
        0,
        2.5,
        Constants.TYPE_UNMODIFIED,
        0,
        Constants.TYPE_SHIFT_TIMESTAMP_NS,
        1,
        Constants.TYPE_UNMODIFIED,
        0,
        Constants.TYPE_DOUBLE,
        0,
        1000.34,
        Constants.TYPE_DOUBLE,
        0,
        -5000.54,
        Constants.BLOCK_TYPE_END,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        1,
    )


def test_write_string():
    with get_output() as d:
        res = ffi.new("otic_column*")
        error = lib.otic_register_column(d.writer, b"test", res)
        assert not error
        column = res[0]
        lib.otic_write_string(column, 1, 0, b"abc", 3)
        lib.otic_write_string(column, 1, 0, b"abc", 3)
        lib.otic_write_string(column, 1, 1, b"abc", 3)
        lib.otic_write_string(column, 1, 1, b"abc" * 100, 300)
        lib.otic_write_string(column, 1, 1, b"xxxxxxxxxx", 10)
    msgcmp(
        [d.result],
        b"TlPa\x00\x01\x00",
        Constants.BLOCK_TYPE_DATA,
        0,
        0,
        1,
        79,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"test",
        Constants.TYPE_STRING,
        0,
        3,
        b"abc",
        Constants.TYPE_UNMODIFIED,
        0,
        Constants.TYPE_SHIFT_TIMESTAMP_NS,
        1,
        Constants.TYPE_UNMODIFIED,
        0,
        Constants.TYPE_STRING,
        0,
        encode_varint_unsigned(300),
        b"abc" * 100,
        Constants.TYPE_STRING,
        0,
        10,
        b"xxxxxxxxxx",
        Constants.BLOCK_TYPE_END,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        1,
    )


def test_write_file(tmp_path):
    p = tmp_path / "foo.fmt"
    print(p)
    w = lib.otic_writer_open_filename_uncompressed(bytes(tmp_path / "foo.fmt"))
    res = ffi.new("otic_column*")
    error = lib.otic_register_column(w, b"test", res)
    assert not error
    column = res[0]
    lib.otic_write_long(column, 1, 0, 2)
    lib.otic_write_long(column, 1, 0, 2)
    lib.otic_write_long(column, 1, 1, 2)
    lib.otic_write_long(column, 1, 1, 1000)
    lib.otic_write_long(column, 1, 1, -5000)
    lib.otic_writer_close(w)
    with open(p, "rb") as f:
        content = f.read()
    msgcmp(
        [content],
        b"TlPa\x00\x01\x00",
        Constants.BLOCK_TYPE_DATA,
        0,
        0,
        0,
        22,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"test",
        2,
        0,
        Constants.TYPE_UNMODIFIED,
        0,
        Constants.TYPE_SHIFT_TIMESTAMP_NS,
        1,
        Constants.TYPE_UNMODIFIED,
        0,
        Constants.TYPE_POS_INT,
        0,
        encode_varint_unsigned(1000),
        Constants.TYPE_NEG_INT,
        0,
        encode_varint_unsigned(~-5000),
        Constants.BLOCK_TYPE_END,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        1,
    )


def test_write_file_and_read_metadata(tmp_path):
    p = tmp_path / "foo.fmt"
    print(p)
    for c in [0, 1]:
        if c:
            w = lib.otic_writer_open_filename(bytes(tmp_path / "foo.fmt"))
        else:
            w = lib.otic_writer_open_filename_uncompressed(bytes(tmp_path / "foo.fmt"))
        res = ffi.new("otic_column*")
        meta = b"\x00 a b"
        smeta = "\x00 a b"
        error = lib.otic_register_column_metadata(w, b"test", meta, len(meta), res)
        assert not error
        column = res[0]
        lib.otic_write_long(column, 1, 0, 2)
        lib.otic_write_long(column, 1, 0, 2)
        lib.otic_write_long(column, 1, 1, 2)
        lib.otic_write_long(column, 1, 1, 1000)
        lib.otic_write_long(column, 1, 1, -5000)
        lib.otic_writer_close(w)
        r = FileReader(p)
        entry = r.read()
        assert entry.columnname == "test"
        assert entry.column_metadata == smeta
        assert entry.epoch == 1
        assert entry.nanoseconds == 0
        assert entry.value == 2
        entry = r.read()
        assert entry.columnname == "test"
        assert entry.column_metadata == smeta
        assert entry.epoch == 1
        assert entry.nanoseconds == 0
        assert entry.value == 2
        entry = r.read()
        assert entry.columnname == "test"
        assert entry.column_metadata == smeta
        assert entry.epoch == 1
        assert entry.nanoseconds == 1
        assert entry.value == 2
        entry = r.read()
        assert entry.columnname == "test"
        assert entry.column_metadata == smeta
        assert entry.epoch == 1
        assert entry.nanoseconds == 1
        assert entry.value == 1000
        entry = r.read()
        assert entry.columnname == "test"
        assert entry.column_metadata == smeta
        assert entry.epoch == 1
        assert entry.nanoseconds == 1
        assert entry.value == -5000
        pytest.raises(StopIteration, r.read)


def _write_c(w, values):
    columns = {}
    for col_name, value, epoch, ns in values:
        if col_name is None:
            lib.otic_writer_flush(w)
            continue
        if col_name not in columns:
            res = ffi.new("otic_column*")
            error = lib.otic_register_column(w, col_name.encode("utf-8"), res)
            assert not error
            columns[col_name] = res[0]
        if type(value) is int:
            lib.otic_write_long(columns[col_name], epoch, ns, value)
        elif type(value) is float:
            lib.otic_write_double(columns[col_name], epoch, ns, value)
        elif type(value) is str:
            value = value.encode("utf-8")
            lib.otic_write_string(
                columns[col_name], epoch, ns, ffi.from_buffer(value), len(value)
            )
        elif value is None:
            lib.otic_write_null(columns[col_name], epoch, ns)
        else:
            assert 0


def _write_c_indexes(w, values):
    columns = {}
    for col_name, value, epoch, ns in values:
        if col_name is None:
            lib.otic_writer_flush(w)
            continue
        if col_name not in columns:
            res = ffi.new("otic_column*")
            error = lib.otic_register_column(w, col_name.encode("utf-8"), res)
            assert not error
            columns[col_name] = lib.otic_column_get_index(res[0])
        if type(value) is int:
            lib.otic_write_long_index(w, columns[col_name], epoch, ns, value)
        elif type(value) is float:
            lib.otic_write_double_index(w, columns[col_name], epoch, ns, value)
        elif type(value) is str:
            value = value.encode("utf-8")
            lib.otic_write_string_index(
                w, columns[col_name], epoch, ns, ffi.from_buffer(value), len(value)
            )
        elif value is None:
            lib.otic_write_null_index(w, columns[col_name], epoch, ns)
        else:
            assert 0


@example(values=[("", 1, 0, 0), ("", 2, 0, 0), ("a", 3, 0, 0)])
@example(
    values=[("", "0000000000000000000000000000000ࠀࠀࠀ𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀𐀀", 0, 0)]
)
@example(values=[("", "\x0000", 0, 0)])
@given(values=values_with_repetitions)
def test_write_and_read(values, tmp_path):
    print(values)
    p = tmp_path / "foo.fmt"
    print(p)
    for indexes in [False, True]:
        for compression in [False, True]:
            if compression:
                w = lib.otic_writer_open_filename(bytes(tmp_path / "foo.fmt"))
            else:
                w = lib.otic_writer_open_filename_uncompressed(
                    bytes(tmp_path / "foo.fmt")
                )

            if indexes:
                _write_c_indexes(w, values)
            else:
                _write_c(w, values)
            lib.otic_writer_close(w)

            with open(p, "rb") as f:
                content = f.read()

            r = Reader(lambda: content)
            for col_name, value, epoch, ns in values:
                if col_name is None:
                    continue
                res = r.read()
                assert res.columnname == col_name
                assert res.epoch == epoch
                assert res.nanoseconds == ns
                assert res.value == value
            pytest.raises(StopIteration, r.read)


def test_write_huge(tmp_path):
    from oticreference.test.test_creader import check_results_fn

    p = bytes(tmp_path / "huge.fmt")
    w = lib.otic_writer_open_filename(p)
    values = []
    for i in range(100000):
        values.append((f"a{i % 20}", i, i, 0))
        values.append((f"a{i % 20}", i + 0.4, i, 1))
        values.append((f"a{i % 20}", str(i + 0.6), i, 1))
    _write_c(w, values)
    lib.otic_writer_close(w)
    check_results_fn(bytes(p), values)


@example(values=[("", "\x0000", 0, 0)])
@given(values=values_with_repetitions)
def test_write_and_write(values, tmp_path):
    p = tmp_path / "foo.fmt"
    w = lib.otic_writer_open_filename_uncompressed(bytes(tmp_path / "foo.fmt"))

    _write_c(w, values)
    lib.otic_writer_close(w)

    with open(p, "rb") as f:
        content = f.read()

    l = []
    w = Writer(l.append, compression=False)
    _write_all(w, values)
    w.close()

    assert b"".join(l) == content


@given(
    offset=strategies.floats(min_value=0.0001, max_value=1.0),
    ts=strategies.floats(min_value=0.0, max_value=1568805739.0),
)
def test_decreasing_timestamps(offset, ts, tmp_path):
    p = tmp_path / "foo.fmt"
    w = lib.otic_writer_open_filename_uncompressed(bytes(p))
    res = ffi.new("otic_column*")
    error = lib.otic_register_column(w, b"test", res)
    c = res[0]
    epoch, ns = convert_float_ts(ts)
    error = lib.otic_write_long(res[0], epoch, ns, 2)
    assert not error
    assume(ts - offset > 0)
    epoch, ns = convert_float_ts(ts - offset)
    error = lib.otic_write_long(res[0], epoch, ns, 2)
    assert error == lib.OTIC_ERROR_TIMESTAMP_DECREASED
