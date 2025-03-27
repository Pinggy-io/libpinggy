import socket
import threading
import struct
import json
import sys

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

def decodeString(bytes, paths = None):
    i = 0
    ln = struct.unpack("@H", bytes[i:][:2])[0]
    i += 2
    data = bytes[i:][:ln].decode("utf-8")
    i += ln
    return data, i

def decodeRaw(bytes, paths = None):
    i = 0
    ln = struct.unpack("@H", bytes[i:][:2])[0]
    i += 2
    data = bytes[i:][:ln]
    i += ln
    return data, i

def decodeInt8(bytes, paths = None):
    i = 0
    val = struct.unpack("@b", bytes[i:][:1])[0]
    i+= 1
    return val, i
def decodeInt16(bytes, paths = None):
    i = 0
    val = struct.unpack("@h", bytes[i:][:2])[0]
    i+=2
    return val, i
def decodeInt32(bytes, paths = None):
    i = 0
    val = struct.unpack("@i", bytes[i:][:4])[0]
    i+=4
    return val, i
def decodeInt64(bytes, paths = None):
    i = 0
    val = struct.unpack("@l", bytes[i:][:8])[0]
    i+=8
    return val, i
# def decodeInt128(bytes, paths = None):
#     i = 0
#     val = struct.upack("@H", bytes[i:][:])[0]
#     i+=
#     return val, i
def decodeUint8(bytes, paths = None):
    i = 0
    val = struct.unpack("@B", bytes[i:][:1])[0]
    i+=1
    return val, i
def decodeUint16(bytes, paths = None):
    i = 0
    val = struct.unpack("@H", bytes[i:][:2])[0]
    i+=2
    return val, i
def decodeUint32(bytes, paths = None):
    i = 0
    val = struct.unpack("@I", bytes[i:][:4])[0]
    i+=4
    return val, i
def decodeUint64(bytes, paths = None):
    i = 0
    val = struct.unpack("@L", bytes[i:][:8])[0]
    i+=8
    return val, i
# def decodeUint128(bytes, paths = None):
#     i = 0
#     val = struct.unpack("@H", bytes[i:][:])[0]
#     i+=
#     return val, i
def decodeFloat32(bytes, paths = None):
    i = 0
    val = struct.unpack("@f", bytes[i:][:4])[0]
    i+=4
    return val, i
def decodeFloat64(bytes, paths = None):
    i = 0
    val = struct.unpack("@d", bytes[i:][:8])[0]
    i+=8
    return val, i
def decodeArray(bytes, paths = None):
    i = 0
    cnt = struct.unpack("@H", bytes[i:][:2])[0]
    i += 2
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
    ValueType_String: decodeString,
    ValueType_Raw: decodeRaw,
    ValueType_Int8: decodeInt8,
    ValueType_Int16: decodeInt16,
    ValueType_Int32: decodeInt32,
    ValueType_Int64: decodeInt64,
    # ValueType_Int128: decodeInt128,
    ValueType_Uint8: decodeUint8,
    ValueType_Uint16: decodeUint16,
    ValueType_Uint32: decodeUint32,
    ValueType_Uint64: decodeUint64,
    # ValueType_Uint128: decodeUint128,
    ValueType_Float32: decodeFloat32,
    ValueType_Float64: decodeFloat64,
    ValueType_Array: decodeArray,
    ValueType_Object: decodeObject,
}

def decodeLit(paths, typ, bytes):
    return decodeFuncs[typ](bytes, paths)

def addValueToPath(object, pathInfo, value):
    path = pathInfo[0]
    # print(path, value)
    if "" == path:
        assert len(object) == 0
        # import pdb; pdb.set_trace()
        return value
    else:
        # if len(pathInfo) < 4:
        #     import pdb; pdb.set_trace()
        # if pathInfo[2] == ValueType_Array:
        #     import pdb; pdb.set_trace()
        rv = object
        pathSections = path[1:].split(".")
        for x in pathSections[:-1]:
            rv = rv.setdefault(x, {})
        rv[pathSections[-1]] = value
        return object

def decodeValue(paths, bytes):
    rootValue = {}
    i = 0
    while i < len(bytes):
        pathId, _i = decodeInt16(bytes[i:])
        i += _i
        pathInfo = paths[pathId]
        typ = pathInfo[2]
        # print("%03d"%pathId, "%03d"%pathInfo[1], pathInfo[0])
        val, _i = decodeLit(paths, typ, bytes[i:])
        i += _i
        # print("%03d"%pathId, "%03d"%pathInfo[1], pathInfo[0], val)
        rootValue = addValueToPath(rootValue, pathInfo, val)
    print(json.dumps(rootValue))

def decodeIncomingData(client):
    print(client)
    paths = {}
    signature = client.recv(256, socket.MSG_WAITALL)
    if not signature.startswith(b"PINGGY"):
        client.close()
        return
    while True:
        lenBytes = client.recv(2, socket.MSG_WAITALL)
        if len(lenBytes) == 0:
            return
        ln = struct.unpack("@H", lenBytes[:2])[0]
        msgBytes = client.recv(ln, socket.MSG_WAITALL)
        hexdump(msgBytes)
        msgType = msgBytes[0]
        msgBytes = msgBytes[1:]
        if msgType == MsgType_Type:
            i = 0
            while i < len(msgBytes):
                pathId = struct.unpack("@H", msgBytes[i:][:2])[0]
                i+=2
                path, _i = decodeString(msgBytes[i:])
                i += _i
                parentId = struct.unpack("@H", msgBytes[i:][:2])[0]
                i += 2
                valueType = msgBytes[i]
                i += 1
                key, _i = decodeString(msgBytes[i:])
                i += _i
                paths[pathId] = [path, parentId, valueType, key, pathId]
            print(paths)
        else:
            decodeValue(paths, msgBytes)

# Function to handle client connections
def handle_client_connection(client_socket, client_address):
    print(f"Accepted connection from {client_address}")
    with client_socket:
        decodeIncomingData(client_socket)
    print(f"Connection from {client_address} closed")

def start_server(host, port):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(5)  # Listen for up to 5 connections
    print(f"Server started on {host}:{port}")

    try:
        while True:
            client_socket, client_address = server_socket.accept()
            if len(sys.argv) > 1:
                handle_client_connection(client_socket, client_address)
            else:
                client_handler = threading.Thread(
                    target=handle_client_connection,
                    args=(client_socket, client_address)
                )
                client_handler.start()
    except KeyboardInterrupt:
        print("Server shutting down")
    finally:
        server_socket.close()

if __name__ == "__main__":
    HOST, PORT = "localhost", 9999
    start_server(HOST, PORT)
