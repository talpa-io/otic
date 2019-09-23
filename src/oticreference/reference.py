import struct
import sys

import zstandard

doublestruct = struct.Struct(">d")
uintstruct = struct.Struct(">I")
ushortstruct = struct.Struct(">H")


def encode_varint_signed(i):
    # https://en.wikipedia.org/wiki/LEB128 signed variant
    res = bytearray()
    more = True
    while more:
        lowest7bits = i & 0b111_1111
        i >>= 7
        if ((i == 0) and (lowest7bits & 0b100_0000) == 0) or (
            (i == -1) and (lowest7bits & 0b100_0000) != 0
        ):
            more = False
        else:
            lowest7bits |= 0b1000_0000
        res.append(lowest7bits)
    return bytes(res)


def decode_varint_signed(b, index=0):
    res = 0
    shift = 0
    while True:
        byte = b[index]
        res = res | ((byte & 0b1111111) << shift)
        index += 1
        shift += 7
        if not (byte & 0b10000000):
            if byte & 0b1000000:
                res |= -1 << shift
            return res, index


def encode_varint_unsigned(i):
    # https://en.wikipedia.org/wiki/LEB128 unsigned variant
    res = bytearray()
    more = True
    if i < 0:
        raise ValueError("only positive numbers supported", i)
    while more:
        lowest7bits = i & 0b111_1111
        i >>= 7
        if i == 0:
            more = False
        else:
            lowest7bits |= 0b1000_0000
        res.append(lowest7bits)
    return bytes(res)


def decode_varint_unsigned(b, index=0):
    res = 0
    shift = 0
    while True:
        byte = b[index]
        res = res | ((byte & 0b1111111) << shift)
        index += 1
        shift += 7
        if not (byte & 0b10000000):
            return res, index


class Constants:
    MAX_DIRECT_INT = 200

    TYPE_NULL = 201
    TYPE_EMPTY_STRING = 204

    TYPE_POS_INT = 210
    TYPE_NEG_INT = 211

    TYPE_DOUBLE = 220

    TYPE_STRING = 230

    TYPE_UNMODIFIED = 231

    TYPE_SHIFT_TIMESTAMP_EPOCH = 240
    TYPE_SHIFT_TIMESTAMP_NS = 241

    TYPE_NAME_ASSIGN = 251
    TYPE_META_ASSIGN = 252

    FILEFORMAT_VERSION = 1

    # Feature Flags
    FEATURE_COMPRESSION = 1

    # Block Header
    BLOCK_TYPE_DATA = 0
    BLOCK_TYPE_RAW = 1
    BLOCK_TYPE_END = 2


class BareWriter(Constants):
    # class for ease of testing

    OUTPUT_BUFFER_LENGTH = 2 * 64 * 1024  # half of BUFSIZE in the C implementation

    def __init__(self, write_block, compression=False):
        self.write_block = write_block
        self.block = bytearray()
        self.columns = {}

        self.compression = compression
        if compression:
            params = zstandard.ZstdCompressionParameters.from_level(22)
            self.compressor = zstandard.ZstdCompressor(compression_params=params)
        else:
            self.compressor = None

        self.first_epoch = self.first_nanoseconds = 0
        self.seen_timestamp_in_block = False
        self.last_epoch = 0
        self.last_nanoseconds = 0

        # statistics
        self.num_rows = 0
        self.total_timestamp_size = 0
        self.num_shift_ns = 0
        self.num_shift_epoch = 0

    def close(self):
        self.flush()
        block_header = bytearray([self.BLOCK_TYPE_END])
        block_header += uintstruct.pack(self.last_epoch)
        block_header += uintstruct.pack(self.last_nanoseconds)
        self.write_block(bytes(block_header))

    def flush(self):
        if self.block:
            header = bytearray([self.BLOCK_TYPE_DATA])
            content = self._compress(bytes(self.block))
            header += uintstruct.pack(len(content))
            header += uintstruct.pack(self.first_epoch)
            header += uintstruct.pack(self.first_nanoseconds)
            self.first_epoch = self.first_nanoseconds = 0
            self.seen_timestamp_in_block = False
            self.write_block(bytes(header) + content)
            self.block.clear()

    def _maybe_flush(self):
        if len(self.block) > self.OUTPUT_BUFFER_LENGTH:
            self.flush()

    def _compress(self, block):
        if self.compression:
            return self.compressor.compress(block)
        return block

    def _write_byte(self, b):
        self.block.append(b)

    def _write_bytes(self, b):
        self.block.extend(b)

    def _write_string(self, s):
        b = s.encode("utf-8")
        if len(b) >= 65536:
            raise ValueError("string too long", s)
        self._write_varuint(len(b))
        self._write_bytes(b)

    def _write_varint(self, i):
        assert isinstance(i, int)
        self._write_bytes(encode_varint_signed(i))

    def _write_varuint(self, i):
        block = self.block
        more = True
        if i < 0:
            raise ValueError("only positive numbers supported", i)
        while more:
            lowest7bits = i & 0b111_1111
            i >>= 7
            if i == 0:
                more = False
            else:
                lowest7bits |= 0b1000_0000
            block.append(lowest7bits)

    def _write_header(self):
        self._write_bytes(b"TlPa")
        self._write_bytes(ushortstruct.pack(self.FILEFORMAT_VERSION))
        features = self.compression * self.FEATURE_COMPRESSION
        self._write_byte(features)  # features

        self.write_block(bytes(self.block))  # write uncompressed!
        self.block.clear()

    def add_column_metadata(self, columnname, metadata):
        if columnname not in self.columns:
            self._assign_column_id(columnname)
        column = self.columns[columnname]
        self._write_byte(self.TYPE_META_ASSIGN)
        self._write_varuint(column.id)
        self._write_string(metadata)

    def write(self, epoch, nanoseconds, columnname, value):
        assert type(epoch) is int
        assert type(nanoseconds) is int
        if columnname not in self.columns:
            self._assign_column_id(columnname)
        column = self.columns[columnname]
        size = len(self.block)
        self._write_timestamp(epoch, nanoseconds)
        self.total_timestamp_size += len(self.block) - size

        size = len(self.block)
        self._write_value(column, value)
        column.total_size_written += len(self.block) - size
        column.num_rows += 1

        self.num_rows += 1

        self._maybe_flush()

    def _write_timestamp(self, epoch, nanoseconds):
        if not self.seen_timestamp_in_block:
            self.first_epoch = self.last_epoch = epoch
            self.first_nanoseconds = self.last_nanoseconds = nanoseconds
            self.seen_timestamp_in_block = True
            return  # encoded in header
        diff = epoch - self.last_epoch
        if diff == 0:
            # same seconds, check nanoseconds
            diff = nanoseconds - self.last_nanoseconds
            if diff == 0:
                return
            if diff < 0:
                raise ValueError("time stamp decreased")
            self._write_byte(self.TYPE_SHIFT_TIMESTAMP_NS)
            self._write_varuint(diff)
            self.last_nanoseconds = nanoseconds
            self.num_shift_ns += 1
            return
        if diff < 0:
            raise ValueError("time stamp decreased")
        self._write_byte(self.TYPE_SHIFT_TIMESTAMP_EPOCH)
        self._write_varuint(diff)
        self._write_varuint(nanoseconds)
        self.last_epoch = epoch
        self.last_nanoseconds = nanoseconds
        self.num_shift_epoch += 1

    def _write_value(self, column, value):
        if (
            value is not None
            and column.last_value == value
            and type(column.last_value) == type(value)
        ):
            self._write_byte(self.TYPE_UNMODIFIED)
            self._write_varuint(column.id)
            return
        column.last_value = value
        if isinstance(value, int):
            if 0 <= value <= self.MAX_DIRECT_INT:
                self._write_byte(value)
                self._write_varuint(column.id)
            elif value >= 0:
                self._write_byte(self.TYPE_POS_INT)
                self._write_varuint(column.id)
                self._write_varuint(value)
            elif value < 0:
                self._write_byte(self.TYPE_NEG_INT)
                self._write_varuint(column.id)
                self._write_varuint(~value)
        elif isinstance(value, float):
            self._write_byte(self.TYPE_DOUBLE)
            self._write_varuint(column.id)
            self._write_bytes(doublestruct.pack(value))
        elif isinstance(value, str):
            self._write_byte(self.TYPE_STRING)
            self._write_varuint(column.id)
            self._write_string(value)
        elif value is None:
            self._write_byte(self.TYPE_NULL)
            self._write_varuint(column.id)
        else:
            raise ValueError("unrecognized type")

    def _assign_column_id(self, columnname):
        column_id = len(self.columns)
        self.columns[columnname] = res = Column(columnname, column_id)
        self._write_column_id_assignment(res)
        return res

    def _write_column_id_assignment(self, column):
        self._write_byte(self.TYPE_NAME_ASSIGN)
        self._write_string(column.name)


class Column:
    def __init__(self, columnname, column_id):
        self.name = columnname
        self.id = column_id
        self.last_value = object()
        self.metadata = None

        self.total_size_written = 0
        self.num_rows = 0

    def __repr__(self):
        return f"Column({self.name}, {self.id})"


class Writer(BareWriter):
    def __init__(self, write_block, compression=False):
        BareWriter.__init__(self, write_block, compression)
        self._write_header()


class FileWriter(Writer):
    def __init__(self, fn, compression=False):
        self.outfile = open(fn, "wb")
        Writer.__init__(self, self.outfile.write, compression)

    def close(self):
        super().close()
        self.outfile.close()


class BareReader(Constants):
    def __init__(self, read_more):
        self.read_more = read_more
        self.buffer = b""
        self.index_buffer = 0

        self.uncompressed_buffer = None
        self.index_uncompressed_buffer = 0

        self.done = False

        self.compression = False

        self.columns = []

        self.last_epoch = 0
        self.last_nanoseconds = 0

    def close(self):
        pass

    def _read_header(self):
        header = self._bare_read(7)
        assert header[:4] == b"TlPa"
        self._version = ushortstruct.unpack(header[4:6])[0]
        assert self._version == self.FILEFORMAT_VERSION
        self._features = header[6]
        if self._features & self.FEATURE_COMPRESSION:
            self.compression = True
            self.decompressor = zstandard.ZstdDecompressor()

    def read(self):
        while 1:
            if (
                self.uncompressed_buffer is None
                or self.index_uncompressed_buffer >= len(self.uncompressed_buffer)
            ):
                self.uncompressed_buffer = None
                self.index_uncompressed_buffer = 0
                more = False
                if not self.done:
                    more = self._read_block()
                if not more:
                    raise StopIteration

            msgtype = self._read_byte()
            if msgtype == self.TYPE_SHIFT_TIMESTAMP_NS:
                self.last_nanoseconds += self._read_varuint()
                continue
            elif msgtype == self.TYPE_SHIFT_TIMESTAMP_EPOCH:
                self.last_epoch += self._read_varuint()
                self.last_nanoseconds = self._read_varuint()
                continue
            elif msgtype == self.TYPE_NAME_ASSIGN:
                name = self._read_string()
                column = Column(name, len(self.columns))
                self.columns.append(column)
                continue
            elif msgtype == self.TYPE_META_ASSIGN:
                column_id = self._read_varuint()
                column = self.columns[column_id]
                column.metadata = self._read_string()
                continue

            column_id = self._read_varuint()
            if msgtype <= self.MAX_DIRECT_INT:
                value = msgtype
            elif msgtype == self.TYPE_POS_INT:
                value = self._read_varuint()
            elif msgtype == self.TYPE_NEG_INT:
                value = ~(self._read_varuint())
            elif msgtype == self.TYPE_STRING:
                value = self._read_string()
            elif msgtype == self.TYPE_UNMODIFIED:
                value = self.columns[column_id].last_value
            elif msgtype == self.TYPE_DOUBLE:
                value = self._read_double()
            elif msgtype == self.TYPE_NULL:
                value = None
            else:
                raise ValueError("unknown message")
            column = self.columns[column_id]
            column.last_value = value

            return Result(
                column.name,
                self.last_epoch,
                self.last_nanoseconds,
                value,
                column.metadata,
            )

    def _bare_read(self, i):
        if self.index_buffer + i > len(self.buffer):
            self.buffer = self.buffer[self.index_buffer :]
            self.index_buffer = 0
            self._read_more()
        res = self.buffer[self.index_buffer : self.index_buffer + i]
        self.index_buffer += i
        return res

    def _read(self, i):
        assert self.index_uncompressed_buffer + i <= len(self.uncompressed_buffer)
        res = self.uncompressed_buffer[
            self.index_uncompressed_buffer : self.index_uncompressed_buffer + i
        ]
        self.index_uncompressed_buffer += i
        return res

    def _read_byte(self):
        return self._read(1)[0]

    def _read_varuint(self):
        res = 0
        shift = 0
        while True:
            byte = self._read_byte()
            res = res | ((byte & 0b1111111) << shift)
            shift += 7
            if not (byte & 0b10000000):
                return res

    def _read_string(self):
        size = self._read_varuint()
        b = self._read(size)
        return b.decode("utf-8")

    def _read_double(self):
        b = self._read(8)
        return doublestruct.unpack(b)[0]

    def _decompress(self, compressed):
        if self.compression:
            return self.decompressor.decompress(compressed)
        else:
            return compressed  # xxx for now

    def _read_more(self):
        b = self.read_more()
        self.buffer += b

    def _read_block(self):
        block_type = self._bare_read(1)[0]
        if block_type == self.BLOCK_TYPE_DATA:
            size = uintstruct.unpack(self._bare_read(4))[0]
            self.last_epoch = uintstruct.unpack(self._bare_read(4))[0]
            self.last_nanoseconds = uintstruct.unpack(self._bare_read(4))[0]
            self.uncompressed_buffer = self._decompress(self._bare_read(size))
            return True
        elif block_type == self.BLOCK_TYPE_END:
            self.done = True
            self.last_epoch = uintstruct.unpack(self._bare_read(4))[0]
            self.last_nanoseconds = uintstruct.unpack(self._bare_read(4))[0]
            return False


class Reader(BareReader):
    def __init__(self, read_more):
        BareReader.__init__(self, read_more)
        self._read_header()


class FileReader(Reader):
    def __init__(self, fn):
        self.infile = open(fn, "rb")
        super().__init__(self.infile.read)

    def close(self):
        super().close()
        self.infile.close()


class Result:
    def __init__(self, columnname, epoch, nanoseconds, value, column_metadata):
        self.columnname = columnname
        self.column_metadata = column_metadata
        self.epoch = epoch
        self.nanoseconds = nanoseconds
        self.value = value

    def __repr__(self):
        return f"<Result columnname={self.columnname}, ts={self.epoch + self.nanoseconds * 1e-9}, value={self.value}>"
