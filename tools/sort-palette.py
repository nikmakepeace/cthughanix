#!/usr/bin/env python3
"""
Sort an indexed PNG palette into luminance-ordered colour bands while preserving
visible image appearance by remapping pixel indices.

Bands, in order:
  1. greyscale / near-neutral
  2. R-dominant
  3. RG-dominant, roughly yellow/orange
  4. G-dominant
  5. GB-dominant, roughly cyan/teal
  6. B-dominant
  7. BR-dominant, roughly magenta/purple
  8. fallback / ambiguous colours

The output remains an indexed/paletted image. The palette is reordered, and image
pixels are remapped so that the rendered image looks the same.
"""

from __future__ import annotations

import argparse
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from PIL import Image


@dataclass(frozen=True)
class PaletteEntry:
    old_index: int
    rgb: tuple[int, int, int]
    alpha: int | None = None

    @property
    def r(self) -> int:
        return self.rgb[0]

    @property
    def g(self) -> int:
        return self.rgb[1]

    @property
    def b(self) -> int:
        return self.rgb[2]

    @property
    def value(self) -> float:
        """Perceptual-ish luminance, suitable for dark-to-light sorting."""
        return 0.2126 * self.r + 0.7152 * self.g + 0.0722 * self.b

    @property
    def chroma(self) -> int:
        return max(self.rgb) - min(self.rgb)

    @property
    def max_channel(self) -> int:
        return max(self.rgb)

    @property
    def min_channel(self) -> int:
        return min(self.rgb)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Sort an indexed PNG palette into greyscale, R, RG, G, GB, B, and BR "
            "bands, each dark-to-light, while preserving image appearance."
        )
    )
    parser.add_argument("input", type=Path, help="Input indexed PNG")
    parser.add_argument("output", type=Path, help="Output indexed PNG")
    parser.add_argument(
        "--neutral-distance",
        type=int,
        default=12,
        help=(
            "Maximum RGB channel spread for a colour to count as greyscale/neutral. "
            "Example: 12 means max(R,G,B) - min(R,G,B) <= 12. Default: 12."
        ),
    )
    parser.add_argument(
        "--co-dominance-distance",
        type=int,
        default=None,
        help=(
            "Maximum distance between the two high channels for two-channel bands "
            "such as RG, GB, and BR. Defaults to --neutral-distance."
        ),
    )
    parser.add_argument(
        "--dominance-margin",
        type=int,
        default=None,
        help=(
            "Minimum margin by which dominant channel(s) must exceed the remaining "
            "channel(s). Defaults to --neutral-distance + 1."
        ),
    )
    parser.add_argument(
        "--include-unused",
        action="store_true",
        help=(
            "Sort all 256 palette slots, including unused colours. By default only "
            "used palette entries are packed first, with unused entries appended."
        ),
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print the planned palette mapping but do not write an output file.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print band counts and index remapping details.",
    )
    return parser.parse_args()


def validate_args(args: argparse.Namespace) -> None:
    for name in ("neutral_distance", "co_dominance_distance", "dominance_margin"):
        value = getattr(args, name)
        if value is not None and not 0 <= value <= 255:
            raise ValueError(f"--{name.replace('_', '-')} must be between 0 and 255")

    if args.co_dominance_distance is None:
        args.co_dominance_distance = args.neutral_distance

    if args.dominance_margin is None:
        args.dominance_margin = args.neutral_distance + 1


def get_used_indices(img: Image.Image) -> set[int]:
    if img.mode != "P":
        raise ValueError(f"Input image must be indexed/paletted mode 'P', got {img.mode!r}")

    used = set()
    # getdata() is simple and reliable for paletted images.
    for pixel in img.getdata():
        if isinstance(pixel, int):
            used.add(pixel)
        else:
            # Defensive only: mode P should yield ints.
            used.add(int(pixel[0]))
    return used


def load_palette_entries(img: Image.Image) -> list[PaletteEntry]:
    palette = img.getpalette()
    if palette is None:
        raise ValueError("Input image has no palette")

    # Pillow may return fewer than 256 RGB triplets. Pad to full PNG palette size.
    palette = list(palette)
    if len(palette) % 3 != 0:
        raise ValueError("Palette data is malformed: RGB triplets are incomplete")

    colour_count = len(palette) // 3
    if colour_count > 256:
        raise ValueError(f"Palette has too many entries: {colour_count}")

    while len(palette) < 256 * 3:
        palette.extend([0, 0, 0])

    alphas = get_palette_alphas(img)

    entries: list[PaletteEntry] = []
    for index in range(256):
        offset = index * 3
        rgb = tuple(palette[offset : offset + 3])
        alpha = alphas[index] if alphas is not None and index < len(alphas) else None
        entries.append(PaletteEntry(old_index=index, rgb=rgb, alpha=alpha))

    return entries


def get_palette_alphas(img: Image.Image) -> list[int] | None:
    """
    Return per-palette alpha values if present.

    PNG indexed transparency is stored in img.info['transparency'] as either:
      - an int: one fully transparent palette index
      - bytes: alpha table for palette entries
    """
    transparency = img.info.get("transparency")
    if transparency is None:
        return None

    alphas = [255] * 256

    if isinstance(transparency, int):
        if 0 <= transparency < 256:
            alphas[transparency] = 0
        return alphas

    if isinstance(transparency, bytes):
        for i, alpha in enumerate(transparency[:256]):
            alphas[i] = alpha
        return alphas

    return None


def classify(entry: PaletteEntry, neutral_distance: int, co_distance: int, margin: int) -> str:
    r, g, b = entry.rgb
    values = {"R": r, "G": g, "B": b}

    # Near-neutral greyscale: no channel is strongly dominant.
    if max(values.values()) - min(values.values()) <= neutral_distance:
        return "grey"

    sorted_channels = sorted(values.items(), key=lambda kv: kv[1], reverse=True)
    high_name, high_value = sorted_channels[0]
    mid_name, mid_value = sorted_channels[1]
    low_name, low_value = sorted_channels[2]

    # Two-channel dominance, e.g. R and G are close and both beat B.
    if high_value - mid_value <= co_distance and mid_value - low_value >= margin:
        pair = "".join(sorted((high_name, mid_name), key="RGB".index))
        if pair == "RG":
            return "rg"
        if pair == "GB":
            return "gb"
        if pair == "BR":
            return "br"

    # Single-channel dominance.
    if high_value - mid_value >= margin:
        if high_name == "R":
            return "r"
        if high_name == "G":
            return "g"
        if high_name == "B":
            return "b"

    return "other"


def hue_tiebreak(entry: PaletteEntry) -> tuple[int, int, int]:
    """
    Deterministic tie-breaker after luminance.

    This avoids unstable-looking reorderings when many colours have similar value.
    """
    return (entry.chroma, entry.max_channel, entry.old_index)


def sort_entries(
    entries: Iterable[PaletteEntry],
    neutral_distance: int,
    co_distance: int,
    margin: int,
) -> tuple[list[PaletteEntry], dict[str, list[PaletteEntry]]]:
    band_order = ["grey", "r", "rg", "g", "gb", "b", "br", "other"]
    bands: dict[str, list[PaletteEntry]] = {name: [] for name in band_order}

    for entry in entries:
        bands[classify(entry, neutral_distance, co_distance, margin)].append(entry)

    for name in band_order:
        bands[name].sort(key=lambda e: (e.value, *hue_tiebreak(e)))

    sorted_palette = [entry for name in band_order for entry in bands[name]]
    return sorted_palette, bands


def remap_image_indices(img: Image.Image, old_to_new: dict[int, int]) -> Image.Image:
    # Point lookup table maps old palette index to new palette index.
    lut = [old_to_new.get(i, i) for i in range(256)]
    return img.point(lut)


def build_palette(entries: list[PaletteEntry]) -> list[int]:
    palette: list[int] = []
    for entry in entries:
        palette.extend(entry.rgb)
    if len(palette) != 256 * 3:
        raise ValueError(f"Expected 256 palette entries, got {len(palette) // 3}")
    return palette


def build_transparency(entries: list[PaletteEntry]) -> bytes | None:
    if not any(entry.alpha is not None and entry.alpha != 255 for entry in entries):
        return None

    alphas = bytes((entry.alpha if entry.alpha is not None else 255) for entry in entries)

    # Trim trailing fully opaque entries because PNG tRNS does not need them.
    return alphas.rstrip(bytes([255]))


def print_report(
    bands: dict[str, list[PaletteEntry]],
    old_to_new: dict[int, int],
    used_indices: set[int],
    include_unused: bool,
) -> None:
    print("Band counts:")
    for name in ["grey", "r", "rg", "g", "gb", "b", "br", "other"]:
        print(f"  {name:5s}: {len(bands[name])}")

    print()
    print("Index mapping:")
    for old_index in sorted(old_to_new):
        used_marker = "used" if old_index in used_indices else "unused"
        print(f"  {old_index:3d} -> {old_to_new[old_index]:3d}  {used_marker}")

    if not include_unused:
        print()
        print("Unused palette entries were appended after used entries.")


def main() -> int:
    args = parse_args()

    try:
        validate_args(args)

        img = Image.open(args.input)
        if img.mode != "P":
            raise ValueError(f"Input image must be indexed/paletted mode 'P', got {img.mode!r}")

        used_indices = get_used_indices(img)
        entries = load_palette_entries(img)

        if args.include_unused:
            sortable_entries = entries
            trailing_entries: list[PaletteEntry] = []
        else:
            sortable_entries = [entry for entry in entries if entry.old_index in used_indices]
            trailing_entries = [entry for entry in entries if entry.old_index not in used_indices]

        sorted_used, bands = sort_entries(
            sortable_entries,
            neutral_distance=args.neutral_distance,
            co_distance=args.co_dominance_distance,
            margin=args.dominance_margin,
        )

        # Keep unused colours deterministic too, but do not let them disturb the
        # compact used palette ranges unless --include-unused is supplied.
        if not args.include_unused:
            trailing_entries, _unused_bands = sort_entries(
                trailing_entries,
                neutral_distance=args.neutral_distance,
                co_distance=args.co_dominance_distance,
                margin=args.dominance_margin,
            )

        new_entries = sorted_used + trailing_entries
        if len(new_entries) != 256:
            raise ValueError(f"Internal error: output palette has {len(new_entries)} entries")

        old_to_new = {entry.old_index: new_index for new_index, entry in enumerate(new_entries)}

        if args.verbose or args.dry_run:
            print_report(bands, old_to_new, used_indices, args.include_unused)

        if args.dry_run:
            return 0

        remapped = remap_image_indices(img, old_to_new)
        remapped.putpalette(build_palette(new_entries))

        transparency = build_transparency(new_entries)
        save_kwargs = {}
        if transparency is not None:
            save_kwargs["transparency"] = transparency

        # Preserve common metadata where safe. Avoid copying old transparency,
        # because the palette indices have changed.
        for key in ("dpi",):
            if key in img.info:
                save_kwargs[key] = img.info[key]

        remapped.save(args.output, format="PNG", **save_kwargs)

        if args.verbose:
            print()
            print(f"Wrote {args.output}")

        return 0

    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
