import multiprocessing
from contextlib import contextmanager

import pytest
from hypothesis import example, given, strategies, settings
from oticreference._internal import ffi, lib
from oticreference.test.test_reference import (
    _pack_test,
    msgcmp,
    values_with_repetitions,
    _write_all,
)
from oticreference.reference import Constants, FileWriter, Writer


def _check_entry(res, value, epoch, nanoseconds, colname, column_metadata=None):
    assert res
    valp = ffi.new("char**")
    if type(value) is int:
        assert lib.otic_result_get_type(res) == lib.OTIC_TYPE_INT
        assert lib.otic_result_get_long(res) == value
    elif type(value) is float:
        assert lib.otic_result_get_type(res) == lib.OTIC_TYPE_DOUBLE
        assert lib.otic_result_get_double(res) == value
    elif type(value) in (bytes, str):
        if type(value) is str:
            value = value.encode("utf-8")
        assert lib.otic_result_get_type(res) == lib.OTIC_TYPE_STRING
        size = lib.otic_result_get_string(res, valp)
        assert size == len(value)
        buf = ffi.buffer(valp[0], size)
        assert bytes(buf) == value
    elif value is None:
        assert lib.otic_result_get_type(res) == lib.OTIC_TYPE_NULL
    else:
        assert 0
    assert lib.otic_result_get_epoch(res) == epoch
    assert lib.otic_result_get_nanoseconds(res) == nanoseconds

    size = lib.otic_result_get_colname(res, valp)
    assert size == len(colname)
    buf = ffi.buffer(valp[0], size)
    assert bytes(buf) == colname.encode("utf-8")

    if column_metadata is not None:
        size = lib.otic_result_get_metadata(res, valp)
        assert size == len(column_metadata)
        buf = ffi.buffer(valp[0], size)
        assert bytes(buf) == column_metadata.encode("utf-8")


@ffi.def_extern()
def python_read_callback(userdata, result, size):
    f = ffi.from_handle(userdata)
    res = f.read(size)
    for i in range(len(res)):
        result[i] = res[i : i + 1]
    return len(res)


def test_reader_writer_int(tmp_path):
    p = tmp_path / "foo.fmt"
    w = FileWriter(p)
    w.write(1, 10, "col1", 4)
    w.write(1, 20, "col1", 4)
    w.write(2, 30, "col1", 5)
    w.flush()
    w.write(2, 50, "col1", 5544499)
    meta = "some amazing data!"
    w.add_column_metadata("col2", meta)
    w.write(5, 50, "col2", -5544499)
    w.close()

    reader = lib.otic_reader_open_filename(bytes(p))
    try:
        res = lib.otic_reader_next(reader)
        _check_entry(res, 4, 1, 10, "col1")

        res = lib.otic_reader_next(reader)
        _check_entry(res, 4, 1, 20, "col1")

        # reader not at the end
        assert lib.otic_reader_closing_epoch(reader) == 0
        assert lib.otic_reader_closing_nanoseconds(reader) == 0

        res = lib.otic_reader_next(reader)
        _check_entry(res, 5, 2, 30, "col1")

        res = lib.otic_reader_next(reader)
        _check_entry(res, 5544499, 2, 50, "col1")
        res = lib.otic_reader_next(reader)
        _check_entry(res, -5544499, 5, 50, "col2", meta)

        res = lib.otic_reader_next(reader)
        assert not res
        assert not lib.otic_reader_geterror(reader)

        assert lib.otic_reader_closing_epoch(reader) == 5
        assert lib.otic_reader_closing_nanoseconds(reader) == 50
    finally:
        lib.otic_reader_close(reader)


def test_reader_writer_double(tmp_path):
    p = tmp_path / "foo.fmt"
    w = FileWriter(p)
    w.write(1, 10, "col1", 4.5)
    w.write(1, 20, "col1", 5.4)
    w.close()

    reader = lib.otic_reader_open_filename(bytes(p))
    try:
        res = lib.otic_reader_next(reader)
        _check_entry(res, 4.5, 1, 10, "col1")

        res = lib.otic_reader_next(reader)
        _check_entry(res, 5.4, 1, 20, "col1")

        res = lib.otic_reader_next(reader)
        assert not res
        assert not lib.otic_reader_geterror(reader)
    finally:
        lib.otic_reader_close(reader)


def test_reader_writer_string(tmp_path):
    p = tmp_path / "foo.fmt"
    w = FileWriter(p)
    w.write(1, 10, "col1", "value1")
    w.write(1, 14, "col1", "value1")
    w.write(1, 20, "col1", "value2")
    w.close()

    reader = lib.otic_reader_open_filename(bytes(p))
    try:

        res = lib.otic_reader_next(reader)
        _check_entry(res, b"value1", 1, 10, "col1")

        res = lib.otic_reader_next(reader)
        _check_entry(res, b"value1", 1, 14, "col1")

        res = lib.otic_reader_next(reader)
        _check_entry(res, b"value2", 1, 20, "col1")

        res = lib.otic_reader_next(reader)
        assert not res
        assert not lib.otic_reader_geterror(reader)
    finally:
        lib.otic_reader_close(reader)


def test_error(tmp_path):
    p = tmp_path / "foo.fmt"
    msg = _pack_test(
        b"TlPa\x00\x01\x00",
        Constants.BLOCK_TYPE_DATA,
        0,
        0,
        0,
        11,
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"test",
        255,  # invalid type code
        1,
        0,
        2,
        0,
        Constants.BLOCK_TYPE_END,
    )
    with open(p, "wb") as f:
        f.write(msg)

    reader = lib.otic_reader_open_filename(bytes(p))
    try:
        valp = ffi.new("char**")

        res = lib.otic_reader_next(reader)
        assert not res
        errc = lib.otic_reader_geterror(reader)
        assert errc == lib.OTIC_ERROR_FILE_CORRUPT
    finally:
        lib.otic_reader_close(reader)


@example(values=[(None, None, None, None)])
@example(values=[(str(i), i, 0, i) for i in range(1000)])  # many columns
@example(
    values=[
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("", 0, 0, 0),
        ("a", 0, 0, 0),
        ("aa", 0, 0, 0),
        ("aaa", 0, 0, 0),
        ("ab", 0, 0, 0),
        ("ac", 0, 0, 0),
        ("ad", 0, 0, 0),
        ("ae", 0, 0, 0),
        ("b", 0, 0, 0),
        ("ba", 0, 0, 0),
        ("c", 0, 0, 0),
        ("d", 0, 0, 0),
        ("e", 0, 0, 0),
        ("f", 0, 0, 0),
        ("g", 0, 0, 0),
        ("h", 0, 0, 0),
        ("i", 0, 0, 0),
        ("j", 0, 0, 0),
    ]
)
@example(
    values=[
        ("a", 0, 0, 0),
        ("a", 0, 0, 0),
        ("b", 0, 0, 0),
        (None, "ignore!", None, None),
        ("b", 0, 0, 0),
        ("a", 0, 0, 0),
        ("b", 0, 0, 0),
        ("b", 0, 0, 0),
    ]
)
@given(values=values_with_repetitions)
@settings(deadline=None)
def test_write_and_read_files(values, tmp_path):
    pa = tmp_path / "foo.fmt"
    for compression in [False, True]:
        w = FileWriter(pa, compression=compression)
        _write_all(w, values)
        w.close()

        if 0:
            p = multiprocessing.Process(
                target=check_results_fn, args=[bytes(pa), values]
            )
            p.start()
            p.join()
            assert p.exitcode == 0
        else:
            check_results_fn(bytes(pa), values)


def check_results_fn(fn, values):
    reader = lib.otic_reader_open_filename(fn)
    return check_results_reader(reader, values)


def check_results_reader(reader, values):
    try:
        ignored = set()
        res = None
        last_colname = None
        for col_name, value, epoch, ns in values:
            if col_name is None:
                if value is None:
                    assert epoch is ns is None
                    continue
                assert value == "ignore!"
                if res is not None:
                    lib.otic_result_ignore_column_from_now_on(res)
                    ignored.add(last_colname)
                continue
            if col_name not in ignored:
                res = lib.otic_reader_next(reader)
                _check_entry(res, value, epoch, ns, col_name)
                last_colname = col_name
        res = lib.otic_reader_next(reader)
        assert not res
        assert lib.otic_reader_geterror(reader) == 0

        if values and epoch is not None:
            assert lib.otic_reader_closing_epoch(reader) == epoch
            assert lib.otic_reader_closing_nanoseconds(reader) == ns
    finally:
        lib.otic_reader_close(reader)


def test_ignore_column(tmp_path):
    pa = tmp_path / "foo.fmt"
    values = [("b", 10, 2, 0), ("b", 15, 3, 0), ("a", 20, 6, 0)]
    for compression in [False, True]:
        w = FileWriter(pa, compression=compression)
        _write_all(w, values)
        w.close()

    reader = lib.otic_reader_open_filename(bytes(pa))
    try:
        res = lib.otic_reader_next(reader)
        _check_entry(res, 10, 2, 0, "b")
        lib.otic_result_ignore_column_from_now_on(res)
        res = lib.otic_reader_next(reader)
        _check_entry(res, 20, 6, 0, "a")
        res = lib.otic_reader_next(reader)
        assert not res
    finally:
        lib.otic_reader_close(reader)


def test_ignore_column_str_bug(tmp_path):
    pa = tmp_path / "foo.fmt"
    values = [("b", 10, 2, 0), ("b", "abc!", 3, 0), ("a", 20, 6, 0)]
    for compression in [False, True]:
        w = FileWriter(pa, compression=compression)
        _write_all(w, values)
        w.close()

    reader = lib.otic_reader_open_filename(bytes(pa))
    try:
        res = lib.otic_reader_next(reader)
        _check_entry(res, 10, 2, 0, "b")
        lib.otic_result_ignore_column_from_now_on(res)
        res = lib.otic_reader_next(reader)
        _check_entry(res, 20, 6, 0, "a")
        res = lib.otic_reader_next(reader)
        assert not res
    finally:
        lib.otic_reader_close(reader)


def test_ignore_column_float(tmp_path):
    pa = tmp_path / "foo.fmt"
    values = [("b", 10.534, 2, 0), ("b", 34.234, 3, 0), ("a", 20, 6, 0)]
    for compression in [False, True]:
        w = FileWriter(pa, compression=compression)
        _write_all(w, values)
        w.close()

    reader = lib.otic_reader_open_filename(bytes(pa))
    try:
        res = lib.otic_reader_next(reader)
        _check_entry(res, 10.534, 2, 0, "b")
        lib.otic_result_ignore_column_from_now_on(res)
        res = lib.otic_reader_next(reader)
        _check_entry(res, 20, 6, 0, "a")
        res = lib.otic_reader_next(reader)
        assert not res
    finally:
        lib.otic_reader_close(reader)


def check_results_content(l, values):
    import io

    f = io.BytesIO(b"".join(l))
    h = ffi.new_handle(f)
    reader = lib.otic_reader_open(lib.python_read_callback, h)
    check_results_reader(reader, values)


@given(values=values_with_repetitions)
def test_open_cb_file(values):
    l = []
    for compression in [False, True]:
        w = Writer(l.append, compression=compression)
        _write_all(w, values)
        w.close()

        if 0:
            p = multiprocessing.Process(target=check_results_content, args=[l, values])
            p.start()
            p.join()
            assert p.exitcode == 0
        else:
            check_results_content(l, values)
