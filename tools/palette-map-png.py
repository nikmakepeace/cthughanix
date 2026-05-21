#!/usr/bin/env python3
"""Generate indexed PNG palette strips from Cthugha .map palette files."""

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


def png_chunk(kind, data):
    body = kind + data
    return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)


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
                rgb = tuple(clamp_channel(int(parts[i])) for i in range(3))
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


def clamp_channel(value):
    if value < 0:
        return 0
    if value > 255:
        return 255
    return value


def write_indexed_png(path, palette):
    plte = bytes(channel for rgb in palette for channel in rgb)
    row = bytes(index for index in range(PALETTE_SIZE) for _ in range(STRIPE_WIDTH))
    image_data = b"".join(b"\x00" + row for _ in range(HEIGHT))

    ihdr = struct.pack(">IIBBBBB", WIDTH, HEIGHT, 8, 3, 0, 0, 0)
    png = b"".join(
        [
            b"\x89PNG\r\n\x1a\n",
            png_chunk(b"IHDR", ihdr),
            png_chunk(b"PLTE", plte),
            png_chunk(b"IDAT", zlib.compress(image_data, level=9)),
            png_chunk(b"IEND", b""),
        ]
    )

    with open(path, "wb") as target:
        target.write(png)


def collect_inputs(inputs):
    paths = []
    for input_path in inputs:
        path = Path(input_path)
        if path.is_dir():
            paths.extend(sorted(path.glob("*.map")))
        else:
            paths.append(path)
    return paths


def output_path_for(input_path, output_dir, used_names):
    stem = input_path.stem
    name = f"{stem}.png"
    suffix = 2

    while name in used_names:
        name = f"{stem}-{suffix}.png"
        suffix += 1

    used_names.add(name)
    return output_dir / name


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Generate 512x256 indexed PNGs from Cthugha .map palettes. "
            "Each palette index is rendered as a 2x256 vertical stripe."
        )
    )
    parser.add_argument(
        "inputs",
        nargs="*",
        default=["map"],
        help="palette .map files or directories containing .map files (default: map)",
    )
    parser.add_argument(
        "-o",
        "--output-dir",
        default="palette-png",
        help="directory for generated PNGs (default: palette-png)",
    )
    args = parser.parse_args()

    inputs = collect_inputs(args.inputs)
    if not inputs:
        print("palette-map-png.py: no input palettes found", file=sys.stderr)
        return 1

    output_dir = Path(args.output_dir)
    os.makedirs(output_dir, exist_ok=True)

    used_names = set()
    generated = 0
    had_errors = False

    for input_path in inputs:
        try:
            palette = read_palette(input_path)
            output_path = output_path_for(input_path, output_dir, used_names)
            write_indexed_png(output_path, palette)
            print(f"{input_path} -> {output_path}")
            generated += 1
        except (OSError, ValueError) as exc:
            print(f"palette-map-png.py: {input_path}: {exc}", file=sys.stderr)
            had_errors = True

    if generated == 0:
        return 1
    if had_errors:
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
