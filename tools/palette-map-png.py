#!/usr/bin/env python3
"""Convert between Cthugha .map palettes and PNG palette-strip images."""

import argparse
import os
import struct
import sys
import zlib
from pathlib import Path


WIDTH = 512
HEIGHT = 256
PALETTE_SIZE = 256
STRIPE_WIDTH = 2
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def png_chunk(kind, data):
    body = kind + data
    return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)


def cthugha_channel(value):
    return value % 256


def read_palette(path):
    palette = []

    with open(path, "r", encoding="utf-8", errors="replace") as source:
        for lineno, line in enumerate(source, 1):
            if len(palette) >= PALETTE_SIZE:
                if line.strip():
                    print(
                        f"palette-map-png.py: warning: {path}: ignoring data after 256 entries",
                        file=sys.stderr,
                    )
                    break
                continue

            parts = line.split()
            if len(parts) < 3:
                print(
                    f"palette-map-png.py: warning: {path}:{lineno}: malformed line, using black",
                    file=sys.stderr,
                )
                palette.append((0, 0, 0))
                continue

            try:
                rgb = tuple(cthugha_channel(int(parts[i])) for i in range(3))
            except ValueError:
                print(
                    f"palette-map-png.py: warning: {path}:{lineno}: malformed line, using black",
                    file=sys.stderr,
                )
                rgb = (0, 0, 0)

            palette.append(rgb)

    if not palette:
        raise ValueError("no palette entries found")

    if len(palette) < PALETTE_SIZE:
        palette.extend([(0, 0, 0)] * (PALETTE_SIZE - len(palette)))

    return palette


def brighten_palette_like_cthugha(palette):
    max_sum = max(r + g + b for r, g, b in palette)
    if max_sum <= 0 or max_sum >= 3 * 255:
        return palette

    factor = (3 * 255) / max_sum
    return [
        (
            min(int(r * factor), 255),
            min(int(g * factor), 255),
            min(int(b * factor), 255),
        )
        for r, g, b in palette
    ]


def write_indexed_png(path, palette):
    plte = bytes(channel for rgb in palette for channel in rgb)
    row = bytes(index for index in range(PALETTE_SIZE) for _ in range(STRIPE_WIDTH))
    image_data = b"".join(b"\x00" + row for _ in range(HEIGHT))

    ihdr = struct.pack(">IIBBBBB", WIDTH, HEIGHT, 8, 3, 0, 0, 0)
    png = b"".join(
        [
            PNG_SIGNATURE,
            png_chunk(b"IHDR", ihdr),
            png_chunk(b"PLTE", plte),
            png_chunk(b"IDAT", zlib.compress(image_data, level=9)),
            png_chunk(b"IEND", b""),
        ]
    )

    with open(path, "wb") as target:
        target.write(png)


def paeth(a, b, c):
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def read_png(path):
    data = Path(path).read_bytes()
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError("not a PNG file")

    pos = len(PNG_SIGNATURE)
    width = height = bit_depth = color_type = interlace = None
    palette = None
    compressed = bytearray()

    while pos + 12 <= len(data):
        length = struct.unpack(">I", data[pos : pos + 4])[0]
        kind = data[pos + 4 : pos + 8]
        chunk = data[pos + 8 : pos + 8 + length]
        pos += 12 + length

        if kind == b"IHDR":
            width, height, bit_depth, color_type, _, _, interlace = struct.unpack(
                ">IIBBBBB", chunk
            )
        elif kind == b"PLTE":
            palette = [tuple(chunk[i : i + 3]) for i in range(0, len(chunk), 3)]
        elif kind == b"IDAT":
            compressed.extend(chunk)
        elif kind == b"IEND":
            break

    if not width or not height:
        raise ValueError("missing IHDR")
    if bit_depth != 8:
        raise ValueError("only 8-bit PNGs are supported")
    if interlace != 0:
        raise ValueError("interlaced PNGs are not supported")

    channels_by_type = {
        0: 1,
        2: 3,
        3: 1,
        4: 2,
        6: 4,
    }
    if color_type not in channels_by_type:
        raise ValueError(f"unsupported PNG color type {color_type}")
    if color_type == 3 and not palette:
        raise ValueError("indexed PNG is missing PLTE")

    channels = channels_by_type[color_type]
    row_bytes = width * channels
    raw = zlib.decompress(bytes(compressed))
    expected = (row_bytes + 1) * height
    if len(raw) < expected:
        raise ValueError("truncated PNG image data")

    pixels = []
    previous = [0] * row_bytes
    offset = 0

    for _ in range(height):
        filter_type = raw[offset]
        offset += 1
        current = list(raw[offset : offset + row_bytes])
        offset += row_bytes

        for i, value in enumerate(current):
            left = current[i - channels] if i >= channels else 0
            up = previous[i]
            up_left = previous[i - channels] if i >= channels else 0

            if filter_type == 1:
                value += left
            elif filter_type == 2:
                value += up
            elif filter_type == 3:
                value += (left + up) // 2
            elif filter_type == 4:
                value += paeth(left, up, up_left)
            elif filter_type != 0:
                raise ValueError(f"unsupported PNG filter {filter_type}")

            current[i] = value & 0xFF

        pixels.append(current)
        previous = current

    return width, height, color_type, channels, palette, pixels


def top_row_palette_from_png(path):
    width, _, color_type, channels, palette, pixels = read_png(path)
    row = pixels[0]
    output = []

    for x in range(min(width, PALETTE_SIZE)):
        sample = row[x * channels : (x + 1) * channels]
        if color_type == 0:
            rgb = (sample[0], sample[0], sample[0])
        elif color_type == 3:
            try:
                rgb = palette[sample[0]]
            except IndexError:
                rgb = (0, 0, 0)
        else:
            rgb = (sample[0], sample[1], sample[2])
        output.append(rgb)

    if not output:
        raise ValueError("PNG has no pixels")

    return output


def write_palette_map(path, palette):
    with open(path, "w", encoding="utf-8") as target:
        for r, g, b in palette:
            target.write(f"{r} {g} {b}\n")


def collect_inputs(inputs):
    paths = []
    for input_path in inputs:
        path = Path(input_path)
        if path.is_dir():
            paths.extend(sorted(path.glob("*.map")))
            paths.extend(sorted(path.glob("*.png")))
        else:
            paths.append(path)
    return paths


def default_output_for(input_path):
    suffix = input_path.suffix.lower()
    if suffix == ".map":
        return input_path.parent / "png" / f"{input_path.stem}.png"
    if suffix == ".png":
        if input_path.parent.name == "png":
            return input_path.parent.parent / f"{input_path.stem}.map"
        return input_path.with_suffix(".map")
    raise ValueError("expected .map or .png input")


def output_path_for(input_path, output_dir, used_names):
    if output_dir is None:
        path = default_output_for(input_path)
    else:
        suffix = ".png" if input_path.suffix.lower() == ".map" else ".map"
        path = output_dir / f"{input_path.stem}{suffix}"

    name = path.name
    suffix_no = 2
    while str(path) in used_names:
        path = path.with_name(f"{path.stem}-{suffix_no}{path.suffix}")
        suffix_no += 1

    used_names.add(str(path))
    return path


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Convert Cthugha .map palettes to indexed PNG strips, or PNG top rows "
            "back to .map palettes."
        )
    )
    parser.add_argument(
        "inputs",
        nargs="*",
        default=["resources/map"],
        help=(
            "palette .map/.png files or directories containing them "
            "(default: resources/map)"
        ),
    )
    parser.add_argument(
        "-o",
        "--output-dir",
        type=Path,
        help=(
            "directory for generated files. By default .map files write to a png/ "
            "subdirectory, and png/foo.png writes back beside that png/ directory."
        ),
    )
    args = parser.parse_args()

    inputs = collect_inputs(args.inputs)
    if not inputs:
        print("palette-map-png.py: no input palettes found", file=sys.stderr)
        return 1

    used_names = set()
    generated = 0
    had_errors = False

    for input_path in inputs:
        try:
            output_path = output_path_for(input_path, args.output_dir, used_names)
            os.makedirs(output_path.parent, exist_ok=True)

            if input_path.suffix.lower() == ".map":
                write_indexed_png(output_path, brighten_palette_like_cthugha(read_palette(input_path)))
            elif input_path.suffix.lower() == ".png":
                write_palette_map(output_path, top_row_palette_from_png(input_path))
            else:
                raise ValueError("expected .map or .png input")

            print(f"{input_path} -> {output_path}")
            generated += 1
        except (OSError, ValueError, zlib.error) as exc:
            print(f"palette-map-png.py: {input_path}: {exc}", file=sys.stderr)
            had_errors = True

    if generated == 0:
        return 1
    if had_errors:
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
