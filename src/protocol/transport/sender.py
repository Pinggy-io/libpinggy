import socket
import struct


ROOT_PATH_ID                = 1

MsgType_Value               = 1
MsgType_Type                = 2

ValueType_Invalid           =  0
ValueType_Array             = 11
ValueType_Object            = 21
ValueType_String            = 31
ValueType_Raw               = 41
ValueType_Int8              = 51
ValueType_Int16             = 52
ValueType_Int32             = 53
ValueType_Int64             = 54
ValueType_Int128            = 55
ValueType_Uint8             = 56
ValueType_Uint16            = 57
ValueType_Uint32            = 58
ValueType_Uint64            = 59
ValueType_Uint128           = 61
ValueType_Float32           = 71
ValueType_Float64           = 81


valueTypeStr = {
 0: "Invalid",
11: "Array",
21: "Object",
31: "String",
41: "Raw",
51: "Int8",
52: "Int16",
53: "Int32",
54: "Int64",
55: "Int128",
56: "Uint8",
57: "Uint16",
58: "Uint32",
59: "Uint64",
61: "Uint128",
71: "Float32",
81: "Float64",
}

def hexdump(data, length=16):
    def format_line(offset, chunk):
        hex_part = ' '.join(f'{byte:02x}' for byte in chunk)
        ascii_part = ''.join(chr(byte) if 32 <= byte <= 126 else '.' for byte in chunk)
        return f'{offset:08x}  {hex_part:<48}  |{ascii_part}|'

    for i in range(0, len(data), length):
        chunk = data[i:i+length]
        print(format_line(i, chunk))


def encodeString(data, paths = None):
    bytes = b""
    raw = data.encode("utf-8")
    return encodeRaw(raw)

def encodeRaw(data, paths = None):
    bytes = b""
    bytes += struct.pack("<H", len(data))
    bytes += data.encode("utf-8")
    return bytes


def encodeInt8(data, paths=None):
    bytes = b""
    bytes += struct.pack("<b", data)
    return bytes
def encodeInt16(data, paths=None):
    bytes = b""
    bytes += struct.pack("<h", data)
    return bytes
def encodeInt32(data, paths=None):
    bytes = b""
    bytes += struct.pack("<i", data)
    return bytes
def encodeInt64(data, paths=None):
    bytes = b""
    bytes += struct.pack("<l", data)
    return bytes
# def encodeInt128(data, paths=None):
#     bytes = b""
#     bytes += struct.pack("<H", data)
#     return bytes
def encodeUint8(data, paths=None):
    bytes = b""
    bytes += struct.pack("<B", data)
    return bytes
def encodeUint16(data, paths=None):
    bytes = b""
    bytes += struct.pack("<H", data)
    return bytes
def encodeUint32(data, paths=None):
    bytes = b""
    bytes += struct.pack("<I", data)
    return bytes
def encodeUint64(data, paths=None):
    bytes = b""
    bytes += struct.pack("<L", data)
    return bytes
# def encodeUint128(data, paths=None):
#     bytes = b""
#     bytes += struct.pack("<H", data)
#     return bytes
def encodeFloat32(data, paths=None):
    bytes = b""
    bytes += struct.pack("<f", data)
    return bytes
def encodeFloat64(data, paths=None):
    bytes = b""
    bytes += struct.pack("<d", data)
    return bytes

def encodeArray(arr, paths = None):
    bytes = b""
    cnt = len(arr)
    bytes += struct.pack("<H", cnt)
    typ = bytes[i]
    i += 1
    values = []
    # print(f"reading {cnt} items of type {typ}({valueTypeStr[typ]})")
    for j in range(cnt):
        val, _i = decodeLit(paths, typ, bytes[i:])
        i += _i
        values.append(val)
    return values, i
def decodeObject(bytes, paths = None):
    object = {}
    array = []
    i = 0
    while True:
        pathId, _i = decodeUint16(bytes[i:])
        i += _i
        if pathId == 1:
            break
        # if pathId < 128:
        #     import pdb; pdb.set_trace();
        pathInfo = paths[pathId]
        typ = pathInfo[2]

        # print("object", pathId, typ, valueTypeStr[typ])
        val, _i = decodeLit(paths, typ, bytes[i:])
        i += _i
        # print("Type: ", type)
        object = addValueToPath(object, pathInfo, val)
    # print(object)
    return object, i

decodeFuncs = {
    ValueType_String:   encodeString,
    ValueType_Raw:      encodeRaw,
    ValueType_Int8:     encodeInt8,
    ValueType_Int16:    encodeInt16,
    ValueType_Int32:    encodeInt32,
    ValueType_Int64:    encodeInt64,
    # ValueType_Int128:   encodeInt128,
    ValueType_Uint8:    encodeUint8,
    ValueType_Uint16:   encodeUint16,
    ValueType_Uint32:   encodeUint32,
    ValueType_Uint64:   encodeUint64,
    # ValueType_Uint128:  encodeUint128,
    ValueType_Float32:  encodeFloat32,
    ValueType_Float64:  encodeFloat64,
    ValueType_Array:    encodeArray,
    ValueType_Object:   encodeObject,
}
