import contextlib

import pytest
from hypothesis import given, settings, strategies
from oticreference.reference import (
    BareReader,
    BareWriter,
    Column,
    Constants,
    FileReader,
    FileWriter,
    Reader,
    Writer,
    decode_varint_signed,
    decode_varint_unsigned,
    doublestruct,
    encode_varint_signed,
    encode_varint_unsigned,
    uintstruct,
)


def test_varint():
    for i in range(100000):
        assert decode_varint_signed(encode_varint_signed(i))[0] == i
        assert decode_varint_unsigned(encode_varint_unsigned(i))[0] == i
        assert decode_varint_signed(encode_varint_signed(-i))[0] == -i


@given(strategies.integers(), strategies.binary())
def test_varint_hypothesis(i, prefix):
    b = encode_varint_signed(i)
    res, pos = decode_varint_signed(b)
    assert res == i
    assert pos == len(b)
    res, pos = decode_varint_signed(prefix + b, len(prefix))
    assert res == i
    assert pos == len(b) + len(prefix)
    if i >= 0:
        b = encode_varint_unsigned(i)
        res, pos = decode_varint_unsigned(b)
        assert res == i
        assert pos == len(b)
        res, pos = decode_varint_unsigned(prefix + b, len(prefix))
        assert res == i
        assert pos == len(b) + len(prefix)


def _pack_test(*args):
    res = bytearray()
    for arg in args:
        if isinstance(arg, int):
            res.append(arg)
        elif isinstance(arg, bytes):
            res.extend(arg)
        elif isinstance(arg, float):
            res.extend(doublestruct.pack(arg))
        else:
            assert 0
    return res


def msgcmp(l, *args):
    b1 = b"".join(l)
    b2 = bytes(_pack_test(*args))
    assert b2 == b1


@contextlib.contextmanager
def check_writer(epoch, ns, *args):
    l = []
    w = BareWriter(l.append)
    yield w
    w.flush()
    args = [
        Constants.BLOCK_TYPE_DATA,
        uintstruct.pack(len(_pack_test(*args))),
        uintstruct.pack(epoch),
        uintstruct.pack(ns),
    ] + list(args)
    msgcmp(l, *args)


def testreader(*args):
    b = _pack_test(*args)
    return BareReader(lambda: b)


def test_write_timestamp():
    with check_writer(
        1,
        1,
        Constants.TYPE_SHIFT_TIMESTAMP_NS,
        4,
        Constants.TYPE_SHIFT_TIMESTAMP_EPOCH,
        2,
        4,
        Constants.TYPE_SHIFT_TIMESTAMP_NS,
        4,
    ) as w:
        w._write_timestamp(1, 1)
        w._write_timestamp(1, 1)
        w._write_timestamp(1, 5)
        w._write_timestamp(3, 4)
        w._write_timestamp(3, 8)
        w.flush()


def test_assign_column_id():
    with check_writer(
        0,
        0,
        Constants.TYPE_NAME_ASSIGN,
        3,
        b"foo",
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"blub",
    ) as w:
        w._assign_column_id("foo")
        w._assign_column_id("blub")


def test_write_int():
    with check_writer(
        0,
        0,
        5,
        0,
        Constants.TYPE_POS_INT,
        0,
        encode_varint_unsigned(1000),
        Constants.TYPE_UNMODIFIED,
        0,
        Constants.TYPE_NEG_INT,
        0,
        encode_varint_unsigned(~-1000),
    ) as w:
        c = Column("col1", 0)
        w._write_value(c, 5)
        w._write_value(c, 1000)
        w._write_value(c, 1000)
        w._write_value(c, -1000)
        w.flush()  # changes nothing, since there is no value


def test_write_double():
    with check_writer(0, 0, Constants.TYPE_DOUBLE, 0, 5.5) as w:
        c = Column("col1", 0)
        w._write_value(c, 5.5)
        w.flush()  # changes nothing, since there is no value


def test_write_string():
    with check_writer(0, 0, Constants.TYPE_STRING, 0, 3, b"abc") as w:
        c = Column("col1", 0)
        w._write_value(c, "abc")
        w.flush()  # changes nothing, since there is no value


def test_write_none():
    with check_writer(0, 0, Constants.TYPE_NULL, 0) as w:
        c = Column("col1", 0)
        w._write_value(c, None)
        w.flush()  # changes nothing, since there is no value


def test_column_metadata():
    with check_writer(
        0,
        0,
        Constants.TYPE_NAME_ASSIGN,
        4,
        b"col1",
        Constants.TYPE_META_ASSIGN,
        0,
        5,
        b"meta!",
    ) as w:
        w.add_column_metadata("col1", "meta!")
        w.flush()  # changes nothing, since there is no value


def test_write_header():
    l = []
    w = BareWriter(l.append)
    w._write_header()
    w.close()
    msgcmp(l, b"TlPa", 0, 1, 0, Constants.BLOCK_TYPE_END, 0, 0, 0, 0, 0, 0, 0, 0)


def test_write_last_timestamp_into_end_marker():
    l = []
    w = BareWriter(l.append)
    w._write_header()
    w.write(1, 1, "col1", 4)
    w.write(1, 2, "col1", 4)
    w.write(2, 0, "col1", 4)
    w.write(2, 5, "col1", 4)
    w.close()
    tail = _pack_test(Constants.BLOCK_TYPE_END, 0, 0, 0, 2, 0, 0, 0, 5)
    assert b"".join(l).endswith(tail)


def test_read_header():
    r = testreader(b"TlPa", 0, 1, 0)
    r._read_header()
    assert r._version == 1
    assert r._features == 0


def test_reader_writer_empty_file():
    l = []
    w = Writer(l.append)
    w.close()

    r = Reader(lambda: b"".join(l))

    with pytest.raises(StopIteration):
        r.read()


def test_reader_writer():
    l = []
    w = Writer(l.append)
    w.write(1, 100000000, "col1", 4)
    w.write(1, 200000000, "col1", 4)
    w.write(1, 300000000, "col1", 5)
    w.write(1, 300000000, "col1", None)
    w.close()
    assert l

    r = Reader(lambda: b"".join(l))

    entry = r.read()
    assert entry.columnname == "col1"
    assert entry.epoch == 1
    assert entry.nanoseconds == 100000000
    assert entry.value == 4
    entry = r.read()
    assert entry.columnname == "col1"
    assert entry.epoch == 1
    assert entry.nanoseconds == 200000000
    assert entry.value == 4
    entry = r.read()
    assert entry.columnname == "col1"
    assert entry.epoch == 1
    assert entry.nanoseconds == 300000000
    assert entry.value == 5
    entry = r.read()
    assert entry.columnname == "col1"
    assert entry.epoch == 1
    assert entry.nanoseconds == 300000000
    assert entry.value is None


def test_reader_writer_metadata():
    l = []
    w = Writer(l.append)
    w.add_column_metadata("col1", "\x00 abc")
    w.write(1, 100000000, "col1", 4)
    w.write(1, 200000000, "col1", 4)
    w.write(1, 300000000, "col1", 5)
    w.close()
    assert l

    r = Reader(lambda: b"".join(l))

    entry = r.read()
    assert entry.columnname == "col1"
    assert entry.column_metadata == "\x00 abc"
    assert entry.epoch == 1
    assert entry.nanoseconds == 100000000
    assert entry.value == 4
    entry = r.read()
    assert entry.columnname == "col1"
    assert entry.column_metadata == "\x00 abc"
    assert entry.epoch == 1
    assert entry.nanoseconds == 200000000
    assert entry.value == 4
    entry = r.read()
    assert entry.columnname == "col1"
    assert entry.column_metadata == "\x00 abc"
    assert entry.epoch == 1
    assert entry.nanoseconds == 300000000
    assert entry.value == 5


def test_compression():
    l1 = []
    w1 = Writer(l1.append, compression=True)
    l2 = []
    w2 = Writer(l2.append, compression=False)
    for i in range(1000):
        w1.write(i, 0, "col1", i | 1)
        w1.write(i, 0, "col2", i * 0.01)
        w2.write(i, 0, "col1", i | 1)
        w2.write(i, 0, "col2", i * 0.01)
    w1.close()
    w2.close()
    assert len(b"".join(l1)) < len(b"".join(l2))


def test_header_uncompressed():
    l = []
    w = Writer(l.append, compression=True)
    for i in range(1000):
        w.write(i, 0, "col1", i | 1)
        w.write(i, 0, "col2", i * 0.01)
    w.close()
    res = b"".join(l)
    assert res.startswith(_pack_test(b"TlPa", 0, 1, 1))


def test_statistics():
    res = []
    w = Writer(res.append)
    for i in range(1000):
        w.write(i, 0, "col1", i | 1)
        w.write(i, 0, "col2", i * 0.01)
    w.close()
    assert w.stats[w.OTIC_STAT_NUM_ROWS] == 2 * 1000
    # timestamps take 3 bytes, but the first ts is in the header
    assert w.stats[w.OTIC_STAT_NUM_BYTES_TS] == 3 * 1000 - 3
    assert w.stats[w.OTIC_STAT_NUM_TS_SHIFT] == 1000 - 1
    assert w.stats[w.OTIC_STAT_NUM_BYTES_VALUES] == (
        100 * 2  # direct ints
        + 400 * 4  # larger ints
        + 500 * 2  # unmodified
        + 1000 * 10  # floats
    )
    assert w.stats[w.OTIC_STAT_NUM_BYTES] == len(b"".join(res))
    assert w.stats[w.OTIC_STAT_NUM_BLOCKS] == 1

    assert w.columns["col1"].stats[w.OTIC_STAT_NUM_ROWS] == 1000
    assert w.columns["col2"].stats[w.OTIC_STAT_NUM_ROWS] == 1000
    assert w.columns["col2"].stats[w.OTIC_STAT_NUM_BYTES] == 1000 * 10
    assert (
        w.columns["col1"].stats[w.OTIC_STAT_NUM_BYTES]
        + w.columns["col2"].stats[w.OTIC_STAT_NUM_BYTES]
    ) == w.stats[w.OTIC_STAT_NUM_BYTES_VALUES]


values = (
    strategies.integers(min_value=-(2 ** 31), max_value=2 ** 31 - 1)
    | strategies.floats(allow_nan=False, allow_infinity=False)
    | strategies.text(max_size=65536)
    | strategies.just(None)
)
time_increments = strategies.floats(min_value=0, max_value=1.0)


def convert_float_ts(ts):
    assert ts >= 0
    epoch = int(ts)
    ns = int((ts - epoch) * 1e9)
    return epoch, ns


def repeat_add_timestamps(values, col_names, data):
    res = []
    ts = data.draw(strategies.floats(min_value=0, max_value=2 ** 29))
    col_names = sorted(col_names)
    for value, count in values:
        for i in range(count):
            ts += data.draw(time_increments)
            col_name = data.draw(strategies.sampled_from(col_names))
            epoch, ns = convert_float_ts(ts)
            res.append((col_name, value, epoch, ns))
    # insert flushes: (None, ) * 4 tuples, ignored when reading
    for i in range(data.draw(strategies.integers(min_value=0, max_value=5))):
        flushpos = data.draw(strategies.integers(min_value=0, max_value=len(res)))
        res.insert(flushpos, (None, None, None, None))
    # insert ignore columns: (None, colname, None, None), ignored when writing
    for i in range(data.draw(strategies.integers(min_value=0, max_value=5))):
        ignorecol = data.draw(strategies.integers(min_value=0, max_value=len(res)))
        res.insert(ignorecol, (None, "ignore!", None, None))
    return res


values_with_repetitions = strategies.builds(
    repeat_add_timestamps,
    strategies.lists(
        strategies.tuples(values, strategies.integers(min_value=1, max_value=10))
    ),
    strategies.sets(
        strategies.text(alphabet="abcdefghijklmopqrstuvwxyz0123456789_ ", max_size=256),
        min_size=1,
    ),
    strategies.data(),
)


def _write_all(w, values):
    for col_name, value, epoch, ns in values:
        if col_name is None:
            w.flush()
        else:
            w.write(epoch, ns, col_name, value)


@given(values_with_repetitions)
def test_write_and_read(values):
    for compression in [False, True]:
        l = []
        w = Writer(l.append, compression=compression)
        _write_all(w, values)
        w.close()

        r = Reader(lambda: b"".join(l))
        for col_name, value, epoch, ns in values:
            if col_name is None:
                continue
            res = r.read()
            assert res.columnname == col_name
            assert res.epoch == epoch
            assert res.nanoseconds == ns
            assert res.value == value
        pytest.raises(StopIteration, r.read)


@given(values=values_with_repetitions)
@settings(deadline=None)
def test_write_and_read_files(values, tmp_path):
    p = tmp_path / "foo.fmt"
    w = FileWriter(p)
    _write_all(w, values)
    w.close()

    r = FileReader(p)
    for col_name, value, epoch, ns in values:
        if col_name is None:
            continue
        res = r.read()
        assert res.columnname == col_name
        assert res.epoch == epoch
        assert res.nanoseconds == ns
        assert res.value == value
    pytest.raises(StopIteration, r.read)
