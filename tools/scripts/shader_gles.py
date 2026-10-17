"""Rewrite a Unity Shader object compiled for OpenGLCore into one for GLES 3.

Unity serializes GLCore and GLES3 shaders the same way (checked against
unity/player/Assets/ShaderProbe built for both). What differs:

- the `platforms` list: 15 (OpenGLCore) becomes 9 (GLES3Plus);
- each program's ShaderGpuProgramType, in the parsed form and in the blob
  entry header: GLCore32 (6) becomes GLES3 (4), GLCore41/43 (7/8) GLES31 (3);
- the per-tier tables: GLCore fills only slot 3, GLES fills slots 0-2;
- the GLSL text: `#version 300 es` (310 es for the 4.20 programs), no
  ARB extension directives, and default precisions.
"""

import re
import struct

import lz4.block

GLCORE, GLES3PLUS = 15, 9
PROGRAM_TYPES = {6: 4, 7: 3, 8: 3}  # GLCore32/41/43 -> GLES3/GLES31

# Every stage gets these: ES has no default precision for the sampler types
# below, and the vertex/fragment pair must agree on float precision.
PRECISION = (
    "precision highp float;\n"
    "precision highp int;\n"
    "precision highp sampler2DArray;\n"
    "precision highp sampler3D;\n"
    "precision highp sampler2DShadow;\n"
    "precision highp samplerCubeShadow;\n"
    "precision highp sampler2DArrayShadow;\n"
    "precision highp isampler2D;\n"
    "precision highp usampler2D;\n"
)

_VERSION = re.compile(r"^#version (\d+)[^\n]*\n", re.M)
_EXTENSION_BLOCK = re.compile(r"^#ifdef (GL_ARB_\w+)\n#extension \1 : \w+\n#endif\n", re.M)
_EXTENSION = re.compile(r"^#extension GL_ARB_\w+ : \w+\n", re.M)
# HLSLcc declares `precise` scratch variables in the 4.20 programs; ES 3.1 has no
# `precise` (it needs gpu_shader5). Dropping it only loses bit-exact invariance.
_PRECISE = re.compile(r"\bprecise\s+")


def convert_glsl(text):
    """GLSL 1.50/4.20 core (both stages in one text, split by #ifdef) to GLSL ES."""
    def version(m):
        es = "310 es" if int(m.group(1)) >= 400 else "300 es"
        return f"#version {es}\n{PRECISION}"
    text = _EXTENSION_BLOCK.sub("", text)
    text = _EXTENSION.sub("", text)
    text = _PRECISE.sub("", text)
    return _VERSION.sub(version, text)


def _align4(n):
    return (n + 3) & ~3


def _find_text(entry):
    """Offset of the length-prefixed GLSL text in a program entry, or None."""
    for marker in (b"#ifdef VERTEX", b"#ifdef FRAGMENT", b"#version"):
        k = entry.find(marker)
        if k >= 4:
            length = struct.unpack_from("<i", entry, k - 4)[0]
            if 0 < length <= len(entry) - k:
                return k
    return None


def convert_entry(entry):
    """One blob entry. Returns it unchanged unless it is a GLCore program."""
    if len(entry) < 8:
        return entry
    program_type = struct.unpack_from("<i", entry, 4)[0]
    if program_type not in PROGRAM_TYPES:
        return entry
    k = _find_text(entry)
    if k is None:
        return entry
    length = struct.unpack_from("<i", entry, k - 4)[0]
    text = entry[k:k + length].decode("utf-8")
    new = convert_glsl(text).encode("utf-8")
    rest = entry[_align4(k + length):]
    out = bytearray(entry[:k - 4])
    struct.pack_into("<i", out, 4, PROGRAM_TYPES[program_type])
    out += struct.pack("<i", len(new)) + new + b"\0" * (_align4(len(new)) - len(new)) + rest
    return bytes(out)


def _convert_segment(raw):
    count = struct.unpack_from("<i", raw, 0)[0]
    table = [struct.unpack_from("<3i", raw, 4 + 12 * i) for i in range(count)]
    entries = [convert_entry(raw[off:off + size]) for off, size, _ in table]
    out = bytearray(struct.pack("<i", count))
    pos = 4 + 12 * count
    for (_, _, segment), entry in zip(table, entries):
        out += struct.pack("<3i", pos, len(entry), segment)
        pos += len(entry)
    for entry in entries:
        out += entry
    return bytes(out)


def _retier(program):
    """GLCore keeps its programs in slot 3 only; GLES expects them per tier (0-2)."""
    subs = program["m_PlayerSubPrograms"]
    params = program["m_ParameterBlobIndices"]
    if len(subs) != 4 or any(subs[:3]) or not subs[3]:
        return
    for sub in subs[3]:
        sub["m_GpuProgramType"] = PROGRAM_TYPES.get(sub["m_GpuProgramType"], sub["m_GpuProgramType"])
    program["m_PlayerSubPrograms"] = [list(subs[3]), list(subs[3]), list(subs[3]), []]
    program["m_ParameterBlobIndices"] = [list(params[3]), list(params[3]), list(params[3]), []]


def convert_shader(tree):
    """Converts a Shader typetree in place. Returns the GLSL texts (for validation)."""
    if tree["platforms"] != [GLCORE]:
        return None
    blob = bytes(tree["compressedBlob"])
    offsets, clens, dlens = [], [], []
    new_blob = bytearray()
    for off, clen, dlen in zip(tree["offsets"][0], tree["compressedLengths"][0], tree["decompressedLengths"][0]):
        raw = lz4.block.decompress(blob[off:off + clen], uncompressed_size=dlen)
        raw = _convert_segment(raw)
        packed = lz4.block.compress(raw, store_size=False)
        offsets.append(len(new_blob))
        clens.append(len(packed))
        dlens.append(len(raw))
        new_blob += packed
    tree["platforms"] = [GLES3PLUS]
    tree["offsets"] = [offsets]
    tree["compressedLengths"] = [clens]
    tree["decompressedLengths"] = [dlens]
    tree["compressedBlob"] = bytes(new_blob)
    for subshader in tree["m_ParsedForm"]["m_SubShaders"]:
        for shader_pass in subshader["m_Passes"]:
            for stage in ("progVertex", "progFragment", "progGeometry", "progHull", "progDomain", "progRayTracing"):
                if shader_pass.get(stage):
                    _retier(shader_pass[stage])
    return glsl_texts(tree)


def glsl_texts(tree):
    """All GLSL texts stored in a Shader typetree's blob."""
    texts = []
    blob = bytes(tree["compressedBlob"])
    for off, clen, dlen in zip(tree["offsets"][0], tree["compressedLengths"][0], tree["decompressedLengths"][0]):
        raw = lz4.block.decompress(blob[off:off + clen], uncompressed_size=dlen)
        count = struct.unpack_from("<i", raw, 0)[0]
        for i in range(count):
            o, size, _ = struct.unpack_from("<3i", raw, 4 + 12 * i)
            entry = raw[o:o + size]
            k = _find_text(entry)
            if k is not None:
                length = struct.unpack_from("<i", entry, k - 4)[0]
                texts.append(entry[k:k + length].decode("utf-8"))
    return texts
