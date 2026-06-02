#!/usr/bin/env python3
"""Convert a monochrome OpenType bitmap font into C++ CP437 glyph rows.

This intentionally supports the compact bitmap-only sfnt shape used by
BmPlus_IBM_VGA_9x14.otb: EBLC index format 2 with EBDT image format 5.
The generated rows are CP437 bytes, with one uint16_t per glyph row.
"""

import argparse
import codecs
import struct
import sys
from pathlib import Path


CP437_CONTROL_CODEPOINTS = {
    0x00: 0x0000,
    0x01: 0x263A,
    0x02: 0x263B,
    0x03: 0x2665,
    0x04: 0x2666,
    0x05: 0x2663,
    0x06: 0x2660,
    0x07: 0x2022,
    0x08: 0x25D8,
    0x09: 0x25CB,
    0x0A: 0x25D9,
    0x0B: 0x2642,
    0x0C: 0x2640,
    0x0D: 0x266A,
    0x0E: 0x266B,
    0x0F: 0x263C,
    0x10: 0x25BA,
    0x11: 0x25C4,
    0x12: 0x2195,
    0x13: 0x203C,
    0x14: 0x00B6,
    0x15: 0x00A7,
    0x16: 0x25AC,
    0x17: 0x21A8,
    0x18: 0x2191,
    0x19: 0x2193,
    0x1A: 0x2192,
    0x1B: 0x2190,
    0x1C: 0x221F,
    0x1D: 0x2194,
    0x1E: 0x25B2,
    0x1F: 0x25BC,
    0x7F: 0x2302,
}


def be_u16(data, offset):
    return struct.unpack_from(">H", data, offset)[0]


def be_i16(data, offset):
    return struct.unpack_from(">h", data, offset)[0]


def be_u32(data, offset):
    return struct.unpack_from(">I", data, offset)[0]


def read_tables(data):
    if len(data) < 12:
        raise ValueError("file is too small to be an sfnt font")

    num_tables = be_u16(data, 4)
    tables = {}
    offset = 12
    for _ in range(num_tables):
        if offset + 16 > len(data):
            raise ValueError("truncated sfnt table directory")
        tag = data[offset:offset + 4].decode("latin1")
        table_offset = be_u32(data, offset + 8)
        table_length = be_u32(data, offset + 12)
        if table_offset + table_length > len(data):
            raise ValueError("table %s extends past end of file" % tag)
        tables[tag] = (table_offset, table_length)
        offset += 16

    return tables


def table_slice(data, tables, tag):
    if tag not in tables:
        raise ValueError("font does not contain required %s table" % tag)
    offset, length = tables[tag]
    return data[offset:offset + length]


def parse_cmap_format4(cmap, subtable_offset):
    if be_u16(cmap, subtable_offset) != 4:
        raise ValueError("internal error: cmap subtable is not format 4")

    seg_count = be_u16(cmap, subtable_offset + 6) // 2
    p = subtable_offset + 14

    end_codes = [be_u16(cmap, p + 2 * i) for i in range(seg_count)]
    p += 2 * seg_count
    p += 2  # reservedPad
    start_codes = [be_u16(cmap, p + 2 * i) for i in range(seg_count)]
    p += 2 * seg_count
    id_deltas = [be_i16(cmap, p + 2 * i) for i in range(seg_count)]
    p += 2 * seg_count
    id_range_offset_positions = [p + 2 * i for i in range(seg_count)]
    id_range_offsets = [be_u16(cmap, p + 2 * i) for i in range(seg_count)]

    mapping = {}
    for i in range(seg_count):
        start = start_codes[i]
        end = end_codes[i]
        if start == 0xFFFF and end == 0xFFFF:
            continue
        for codepoint in range(start, end + 1):
            offset = id_range_offsets[i]
            if offset == 0:
                glyph = (codepoint + id_deltas[i]) & 0xFFFF
            else:
                glyph_offset = (id_range_offset_positions[i] + offset
                                + 2 * (codepoint - start))
                if glyph_offset + 2 > len(cmap):
                    continue
                glyph = be_u16(cmap, glyph_offset)
                if glyph != 0:
                    glyph = (glyph + id_deltas[i]) & 0xFFFF
            if glyph != 0:
                mapping[codepoint] = glyph

    return mapping


def parse_cmap(data, tables):
    cmap = table_slice(data, tables, "cmap")
    if len(cmap) < 4:
        raise ValueError("cmap table is truncated")

    record_count = be_u16(cmap, 2)
    records = []
    for i in range(record_count):
        offset = 4 + i * 8
        platform_id = be_u16(cmap, offset)
        encoding_id = be_u16(cmap, offset + 2)
        subtable_offset = be_u32(cmap, offset + 4)
        if subtable_offset + 2 > len(cmap):
            continue
        fmt = be_u16(cmap, subtable_offset)
        records.append((platform_id, encoding_id, fmt, subtable_offset))

    preferred = [
        (3, 1, 4),  # Windows Unicode BMP
        (0, 3, 4),  # Unicode BMP
        (0, 0, 4),
    ]
    for want in preferred:
        for platform_id, encoding_id, fmt, subtable_offset in records:
            if (platform_id, encoding_id, fmt) == want:
                return parse_cmap_format4(cmap, subtable_offset)

    for _, _, fmt, subtable_offset in records:
        if fmt == 4:
            return parse_cmap_format4(cmap, subtable_offset)

    raise ValueError("no supported cmap format 4 subtable found")


def unpack_bit_aligned_rows(raw, width, height):
    rows = []
    for y in range(height):
        row = 0
        for x in range(width):
            bit_index = y * width + x
            byte = raw[bit_index // 8]
            if byte & (0x80 >> (bit_index % 8)):
                row |= 1 << (width - 1 - x)
        rows.append(row)
    return rows


def parse_bitmap_strike(data, tables):
    eblc = table_slice(data, tables, "EBLC")
    ebdt = table_slice(data, tables, "EBDT")

    if len(eblc) < 8:
        raise ValueError("EBLC table is truncated")
    if be_u32(eblc, 0) != 0x00020000:
        raise ValueError("unsupported EBLC version")

    size_count = be_u32(eblc, 4)
    if size_count < 1:
        raise ValueError("font contains no bitmap strikes")

    size_offset = 8
    if size_offset + 48 > len(eblc):
        raise ValueError("EBLC bitmap size table is truncated")

    index_array_offset = be_u32(eblc, size_offset)
    index_tables_size = be_u32(eblc, size_offset + 4)
    subtable_count = be_u32(eblc, size_offset + 8)
    start_glyph = be_u16(eblc, size_offset + 40)
    end_glyph = be_u16(eblc, size_offset + 42)
    bit_depth = eblc[size_offset + 46]

    if bit_depth != 1:
        raise ValueError("only monochrome bitmap strikes are supported")

    glyph_rows = {}
    width = None
    height = None

    for entry_index in range(subtable_count):
        entry_offset = index_array_offset + entry_index * 8
        if entry_offset + 8 > len(eblc):
            raise ValueError("EBLC index subtable array is truncated")

        first = be_u16(eblc, entry_offset)
        last = be_u16(eblc, entry_offset + 2)
        additional_offset = be_u32(eblc, entry_offset + 4)
        subtable_offset = index_array_offset + additional_offset
        if subtable_offset + 20 > len(eblc):
            raise ValueError("EBLC index subtable is truncated")

        index_format = be_u16(eblc, subtable_offset)
        image_format = be_u16(eblc, subtable_offset + 2)
        image_data_offset = be_u32(eblc, subtable_offset + 4)

        if index_format != 2 or image_format != 5:
            raise ValueError(
                "unsupported bitmap subtable: index format %d, image format %d"
                % (index_format, image_format))

        image_size = be_u32(eblc, subtable_offset + 8)
        glyph_height = eblc[subtable_offset + 12]
        glyph_width = eblc[subtable_offset + 13]

        if width is None:
            width = glyph_width
            height = glyph_height
        elif width != glyph_width or height != glyph_height:
            raise ValueError("mixed glyph sizes are not supported")

        for glyph_id in range(first, last + 1):
            image_offset = image_data_offset + (glyph_id - first) * image_size
            image = ebdt[image_offset:image_offset + image_size]
            if len(image) != image_size:
                raise ValueError("EBDT glyph image is truncated")
            glyph_rows[glyph_id] = unpack_bit_aligned_rows(
                image, glyph_width, glyph_height)

    if width is None or height is None:
        raise ValueError("no supported bitmap glyphs found")

    return width, height, start_glyph, end_glyph, glyph_rows


def cp437_codepoint(byte_value):
    if byte_value in CP437_CONTROL_CODEPOINTS:
        return CP437_CONTROL_CODEPOINTS[byte_value]
    return ord(codecs.decode(bytes([byte_value]), "cp437"))


def cpp_char_comment(byte_value):
    if 32 <= byte_value <= 126 and byte_value not in (ord("'"), ord("\\")):
        return "'%s'" % chr(byte_value)
    if byte_value == ord("'"):
        return "'\\''"
    if byte_value == ord("\\"):
        return "'\\\\'"
    return "U+%04X" % cp437_codepoint(byte_value)


def generate_cpp(source_label, font_name, width, height, cp437_rows):
    lines = []
    lines.append("// Generated by tools/otb_to_bitmap_font.py; do not edit by hand.")
    lines.append("// Source font: %s." % font_name)
    lines.append("// Source file: %s." % source_label)
    lines.append("// Font attribution: VileR, http://int10h.org.")
    lines.append("// License: Creative Commons Attribution-ShareAlike 4.0 International.")
    lines.append("// Row masks use bit %d as the leftmost pixel of each 9-pixel row." % (width - 1))
    lines.append("")
    lines.append('#include "BitmapFont.h"')
    lines.append("")
    lines.append("static const uint16_t dosVga9x14GlyphRows[256 * %d] = {" % height)
    for byte_value, rows in enumerate(cp437_rows):
        lines.append("    // 0x%02X %s" % (byte_value, cpp_char_comment(byte_value)))
        line = "    "
        for i, row in enumerate(rows):
            part = "0x%03X" % row
            if byte_value != 255 or i != len(rows) - 1:
                part += ","
            if len(line) + len(part) + 1 > 96:
                lines.append(line.rstrip())
                line = "    "
            line += part + " "
        lines.append(line.rstrip())
    lines.append("};")
    lines.append("")
    lines.append("const BitmapFont& dosVga9x14Font() {")
    lines.append("    static const BitmapFont font = {")
    lines.append('        "BmPlus IBM VGA 9x14",')
    lines.append("        %d," % width)
    lines.append("        %d," % height)
    lines.append("        dosVga9x14GlyphRows")
    lines.append("    };")
    lines.append("")
    lines.append("    return font;")
    lines.append("}")
    lines.append("")
    return "\n".join(lines)


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--source-label")
    parser.add_argument("--font-name", default="BmPlus IBM VGA 9x14")
    args = parser.parse_args(argv)

    data = args.input.read_bytes()
    tables = read_tables(data)
    cmap = parse_cmap(data, tables)
    width, height, _, _, glyph_rows = parse_bitmap_strike(data, tables)

    fallback_glyph = cmap.get(ord("?"))
    if fallback_glyph is None or fallback_glyph not in glyph_rows:
        fallback_rows = [0] * height
    else:
        fallback_rows = glyph_rows[fallback_glyph]

    cp437_rows = []
    missing = []
    for byte_value in range(256):
        if byte_value == 0:
            cp437_rows.append([0] * height)
            continue
        codepoint = cp437_codepoint(byte_value)
        glyph_id = cmap.get(codepoint)
        rows = glyph_rows.get(glyph_id)
        if rows is None:
            missing.append((byte_value, codepoint))
            rows = fallback_rows
        cp437_rows.append(rows)

    if missing:
        details = ", ".join("0x%02X/U+%04X" % item for item in missing[:16])
        raise ValueError("missing CP437 glyph mappings: %s" % details)

    source_label = args.source_label if args.source_label else str(args.input)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(generate_cpp(
        source_label, args.font_name, width, height, cp437_rows))

    print("wrote %s: 256 CP437 glyphs, %dx%d" % (args.output, width, height))


if __name__ == "__main__":
    try:
        main(sys.argv[1:])
    except Exception as exc:
        print("%s: %s" % (Path(sys.argv[0]).name, exc), file=sys.stderr)
        sys.exit(1)
