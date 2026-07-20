#!/usr/bin/env python3
"""Generates the .onnx test fixtures by hand-packing the protobuf wire format.

No dependencies (no onnx, no protobuf). Field numbers follow onnx.proto.
Run from anywhere: fixtures are written next to this script.
"""

import os
import struct

OUT_DIR = os.path.dirname(os.path.abspath(__file__))

FLOAT = 1  # TensorProto.DataType.FLOAT
ATTR_INT = 2  # AttributeProto.AttributeType.INT


def varint(n: int) -> bytes:
    n &= (1 << 64) - 1
    out = bytearray()
    while True:
        byte = n & 0x7F
        n >>= 7
        if n:
            out.append(byte | 0x80)
        else:
            out.append(byte)
            return bytes(out)


def key(field: int, wire_type: int) -> bytes:
    return varint((field << 3) | wire_type)


def ld(field: int, payload: bytes) -> bytes:
    return key(field, 2) + varint(len(payload)) + payload


def string(field: int, text: str) -> bytes:
    return ld(field, text.encode())


def vint(field: int, n: int) -> bytes:
    return key(field, 0) + varint(n)


def dim_value(v: int) -> bytes:
    return vint(1, v)


def dim_param(name: str) -> bytes:
    return string(2, name)


def value_info(name: str, dims) -> bytes:
    """dims entries: int for a fixed size, str for a dynamic dim_param."""
    shape = b"".join(
        ld(1, dim_param(d) if isinstance(d, str) else dim_value(d)) for d in dims
    )
    tensor_type = vint(1, FLOAT) + ld(2, shape)
    return string(1, name) + ld(2, ld(1, tensor_type))


def initializer(name: str, dims) -> bytes:
    count = 1
    for d in dims:
        count *= d
    packed_dims = ld(1, b"".join(varint(d) for d in dims))
    raw_data = ld(9, struct.pack(f"<{count}f", *([0.0] * count)))
    return packed_dims + vint(2, FLOAT) + string(8, name) + raw_data


def attr_int(name: str, value: int) -> bytes:
    return string(1, name) + vint(3, value) + vint(20, ATTR_INT)


def node(op: str, inputs, outputs, name: str = "", attrs=()) -> bytes:
    out = b"".join(string(1, i) for i in inputs)
    out += b"".join(string(2, o) for o in outputs)
    if name:
        out += string(3, name)
    out += string(4, op)
    out += b"".join(ld(5, a) for a in attrs)
    return out


def graph(name: str, nodes, initializers, inputs, outputs) -> bytes:
    out = b"".join(ld(1, n) for n in nodes)
    out += string(2, name)
    out += b"".join(ld(5, t) for t in initializers)
    out += b"".join(ld(11, i) for i in inputs)
    out += b"".join(ld(12, o) for o in outputs)
    return out


def model(g: bytes) -> bytes:
    ir_version = vint(1, 8)
    opset_import = ld(8, vint(2, 13))
    return ir_version + ld(7, g) + opset_import


def write(filename: str, g: bytes) -> None:
    path = os.path.join(OUT_DIR, filename)
    with open(path, "wb") as f:
        f.write(model(g))
    print(f"wrote {path}")


def gemm(x, w, b, y, name, trans_b=1):
    return node("Gemm", [x, w, b], [y], name, [attr_int("transB", trans_b)])


# Valid two-layer MLP: 5 -> relu 10 -> softmax 3.
write(
    "test.onnx",
    graph(
        "SimpleMLP",
        [
            gemm("a", "w1", "b1", "h1", "l1"),
            node("Relu", ["h1"], ["h2"]),
            gemm("h2", "w2", "b2", "h3", "l2"),
            node("Softmax", ["h3"], ["y"]),
        ],
        [initializer("w1", [10, 5]), initializer("b1", [10]),
         initializer("w2", [3, 10]), initializer("b2", [3])],
        [value_info("a", ["N", 5])],
        [value_info("y", ["N", 3])],
    ),
)

# Valid three-layer chain; second Gemm uses transB=0 ([in, out] weights),
# last layer has no activation node (lowered to linear).
write(
    "valid_chain.onnx",
    graph(
        "Chain",
        [
            gemm("x", "w1", "b1", "h1", "l1"),
            node("Relu", ["h1"], ["h2"]),
            gemm("h2", "w2", "b2", "h3", "l2", trans_b=0),
            node("Relu", ["h3"], ["h4"]),
            gemm("h4", "w3", "b3", "y", "l3"),
        ],
        [initializer("w1", [16, 8]), initializer("b1", [16]),
         initializer("w2", [16, 4]), initializer("b2", [4]),
         initializer("w3", [2, 4]), initializer("b3", [2])],
        [value_info("x", ["N", 8])],
        [value_info("y", ["N", 2])],
    ),
)

# Weights initializer shares the layer's name -> sema "duplicate name".
write(
    "invalid_dup.onnx",
    graph(
        "DupNames",
        [
            gemm("a", "l1", "b1", "h1", "l1"),
            node("Relu", ["h1"], ["y"]),
        ],
        [initializer("l1", [10, 5]), initializer("b1", [10])],
        [value_info("a", ["N", 5])],
        [value_info("y", ["N", 10])],
    ),
)

# Bias size 8 disagrees with weights output size 10 -> sema "size mismatch".
write(
    "invalid_mismatch.onnx",
    graph(
        "Mismatch",
        [
            gemm("a", "w1", "b1", "h1", "l1"),
            node("Relu", ["h1"], ["y"]),
        ],
        [initializer("w1", [10, 5]), initializer("b1", [8])],
        [value_info("a", ["N", 5])],
        [value_info("y", ["N", 10])],
    ),
)

# Rank-2 bias -> sema "must have rank 1".
write(
    "invalid_rank.onnx",
    graph(
        "BadRank",
        [
            gemm("a", "w1", "b1", "h1", "l1"),
            node("Relu", ["h1"], ["y"]),
        ],
        [initializer("w1", [10, 5]), initializer("b1", [10, 1])],
        [value_info("a", ["N", 5])],
        [value_info("y", ["N", 10])],
    ),
)

# Sigmoid parses as an activation but sema rejects it -> "unknown activation".
write(
    "invalid_activation.onnx",
    graph(
        "BadActivation",
        [
            gemm("a", "w1", "b1", "h1", "l1"),
            node("Sigmoid", ["h1"], ["y"]),
        ],
        [initializer("w1", [10, 5]), initializer("b1", [10])],
        [value_info("a", ["N", 5])],
        [value_info("y", ["N", 10])],
    ),
)

# Graph with no nodes -> sema "has no layers".
write(
    "invalid_nolayers.onnx",
    graph("Empty", [], [], [value_info("a", ["N", 5])], [value_info("a", ["N", 5])]),
)

# Conv is not in the supported subset -> parse "unsupported op".
write(
    "invalid_op.onnx",
    graph(
        "BadOp",
        [
            node("Conv", ["a", "w1"], ["y"], "c1"),
        ],
        [initializer("w1", [4, 1, 3, 3])],
        [value_info("a", ["N", 1, 8, 8])],
        [value_info("y", ["N", 4, 6, 6])],
    ),
)
