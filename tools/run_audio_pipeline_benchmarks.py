#!/usr/bin/env python3
import argparse
import datetime as dt
import hashlib
import json
import os
import platform
import shutil
import statistics
import subprocess
import sys
from pathlib import Path


SCHEMA = "cthughanix.audio-pipeline-benchmarks.v3"
DEFAULT_REPETITIONS = 40
DEFAULT_WARMUP_TIME = "1.0"
DEFAULT_MIN_TIME = "0.03s"
AUDIO_SLICE_MS = 10


def run(cmd, cwd=None, check=True):
    result = subprocess.run(
        cmd,
        cwd=cwd,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if check and result.returncode != 0:
        sys.stderr.write(result.stdout)
        sys.stderr.write(result.stderr)
        raise SystemExit(result.returncode)
    return result


def git_value(args, cwd):
    result = run(["git"] + args, cwd=cwd, check=False)
    if result.returncode != 0:
        return ""
    return result.stdout.strip()


def sha256(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def benchmark_binary(build_dir):
    candidates = [
        build_dir / "tests" / "benchmarks" / "audio_pipeline_bench",
        build_dir / "audio_pipeline_bench",
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise SystemExit(f"audio_pipeline_bench was not found under {build_dir}")


def benchmark_arg_present(args, name):
    prefix = name + "="
    return any(arg == name or arg.startswith(prefix) for arg in args)


def benchmark_name(name):
    return name.split("/repeats:", 1)[0]


def time_to_ms(value, unit):
    if unit == "ns":
        return value / 1000000.0
    if unit == "us":
        return value / 1000.0
    if unit == "ms":
        return value
    if unit == "s":
        return value * 1000.0
    return value


def nearest_rank_percentile(values, percentile):
    import math

    if not values:
        return 0.0
    rank = int(math.ceil((percentile / 100.0) * len(values)))
    rank = min(max(rank, 1), len(values))
    return sorted(values)[rank - 1]


def spread(values):
    if not values:
        return {
            "samples": 0,
            "mean_ms": 0.0,
            "sd_ms": 0.0,
            "cv_percent": 0.0,
            "min_ms": 0.0,
            "max_ms": 0.0,
            "median_ms": 0.0,
            "p99_ms": 0.0,
        }
    mean = statistics.fmean(values)
    sd = statistics.stdev(values) if len(values) > 1 else 0.0
    return {
        "samples": len(values),
        "mean_ms": mean,
        "sd_ms": sd,
        "cv_percent": (sd / mean * 100.0) if mean else 0.0,
        "min_ms": min(values),
        "max_ms": max(values),
        "median_ms": statistics.median(values),
        "p99_ms": nearest_rank_percentile(values, 99.0),
    }


def load_benchmark_metrics(path):
    with path.open("r", encoding="utf-8") as f:
        data = json.load(f)
    grouped = {}
    for entry in data.get("benchmarks", []):
        if entry.get("run_type") == "aggregate":
            continue
        name = benchmark_name(entry.get("name", ""))
        unit = entry.get("time_unit", "")
        grouped.setdefault(name, []).append(
            {
                "real_time_ms": time_to_ms(entry.get("real_time", 0.0), unit),
                "cpu_time_ms": time_to_ms(entry.get("cpu_time", 0.0), unit),
                "items_per_second": entry.get("items_per_second"),
            }
        )

    rows = []
    for name, samples in grouped.items():
        real_times = [sample["real_time_ms"] for sample in samples]
        cpu_times = [sample["cpu_time_ms"] for sample in samples]
        item_rates = [
            sample["items_per_second"]
            for sample in samples
            if sample.get("items_per_second") is not None
        ]
        row = {
            "name": name,
            "total_samples": len(samples),
            "real_time": spread(real_times),
            "cpu_time": spread(cpu_times),
            "items_per_second_mean": statistics.fmean(item_rates) if item_rates else None,
        }
        rows.append(row)
    rows.sort(key=lambda row: row["name"])
    return rows


def cmake_cache_value(build_dir, key):
    cache = build_dir / "CMakeCache.txt"
    if not cache.exists():
        return ""
    prefix = key + ":"
    with cache.open("r", encoding="utf-8", errors="replace") as f:
        for line in f:
            if line.startswith(prefix):
                return line.split("=", 1)[1].strip()
    return ""


def write_summary(path, manifest, rows):
    with path.open("w", encoding="utf-8") as f:
        f.write("# Audio Pipeline Benchmark Summary\n\n")
        f.write(f"- Schema: `{manifest['schema']}`\n")
        f.write(f"- Timestamp: `{manifest['timestamp_utc']}`\n")
        f.write(f"- Git: `{manifest['git_commit']}`")
        if manifest["git_dirty"]:
            f.write(" dirty")
        f.write("\n")
        f.write(f"- Build type: `{manifest['build_type']}`\n")
        f.write(f"- C++ compiler: `{manifest['cxx_compiler']}`\n")
        f.write(f"- Repetitions: `{manifest['benchmark_repetitions']}`\n")
        f.write(f"- Timed warmup: `{manifest['benchmark_warmup_time']} s`\n")
        f.write(f"- Audio slice: `{manifest['audio_slice_ms']} ms`\n")
        f.write("- Timing unit: `ms`\n")
        f.write(f"- Build dir: `{manifest['build_dir']}`\n")
        f.write(f"- Benchmark binary: `{manifest['benchmark_binary']}`\n\n")
        f.write("| Benchmark | Samples | Mean ms | SD ms | CV % | Min ms | Max ms | Median ms | P99 ms | Items/s |\n")
        f.write("| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |\n")
        for row in rows:
            timing = row["real_time"]
            items = row["items_per_second_mean"]
            items_text = "" if items is None else f"{items:.2f}"
            f.write(
                f"| `{row['name']}` | {timing['samples']} | "
                f"{timing['mean_ms']:.6f} | {timing['sd_ms']:.6f} | "
                f"{timing['cv_percent']:.2f} | {timing['min_ms']:.6f} | "
                f"{timing['max_ms']:.6f} | {timing['median_ms']:.6f} | "
                f"{timing['p99_ms']:.6f} | {items_text} |\n"
            )

        f.write("\nCPU-time spread metrics are available in `spread-metrics.json`.\n")


def main():
    parser = argparse.ArgumentParser(
        description="Run and archive CthughaNix audio pipeline benchmarks."
    )
    parser.add_argument("--build-dir", default="build")
    parser.add_argument("--source-dir", default=None)
    parser.add_argument("--out-dir", default=None)
    parser.add_argument("--benchmark-arg", action="append", default=[])
    parser.add_argument("--repetitions", type=int, default=DEFAULT_REPETITIONS)
    parser.add_argument("--warmup-time", default=DEFAULT_WARMUP_TIME)
    args = parser.parse_args()

    build_dir = Path(args.build_dir).resolve()
    source_dir = Path(args.source_dir).resolve() if args.source_dir else Path.cwd()
    binary = benchmark_binary(build_dir)
    timestamp = dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H%M%SZ")
    git_commit = git_value(["rev-parse", "--short=12", "HEAD"], source_dir) or "unknown"
    dirty = bool(git_value(["status", "--porcelain"], source_dir))
    run_id = f"{timestamp}-{git_commit}{'-dirty' if dirty else ''}"
    out_root = Path(args.out_dir).resolve() if args.out_dir else (
        build_dir / "test-results" / "audio-pipeline"
    )
    out_dir = out_root / run_id
    out_dir.mkdir(parents=True, exist_ok=True)

    benchmark_json = out_dir / "google-benchmark.json"
    console_log = out_dir / "console.log"
    fixture_dir = source_dir / "tests" / "fixtures" / "audio"
    fixtures = []
    if fixture_dir.exists():
        for fixture in sorted(fixture_dir.glob("*.wav")):
            fixtures.append(
                {
                    "file": str(fixture.relative_to(source_dir)),
                    "bytes": fixture.stat().st_size,
                    "sha256": sha256(fixture),
                }
            )

    benchmark_args = list(args.benchmark_arg)
    if not benchmark_arg_present(benchmark_args, "--benchmark_repetitions"):
        benchmark_args.append(f"--benchmark_repetitions={args.repetitions}")
    if not benchmark_arg_present(benchmark_args, "--benchmark_min_warmup_time"):
        benchmark_args.append(f"--benchmark_min_warmup_time={args.warmup_time}")
    if not benchmark_arg_present(benchmark_args, "--benchmark_report_aggregates_only"):
        benchmark_args.append("--benchmark_report_aggregates_only=false")
    if not benchmark_arg_present(benchmark_args, "--benchmark_time_unit"):
        benchmark_args.append("--benchmark_time_unit=ms")
    if not benchmark_arg_present(benchmark_args, "--benchmark_min_time"):
        benchmark_args.append(f"--benchmark_min_time={DEFAULT_MIN_TIME}")

    cmd = [
        str(binary),
        "--benchmark_format=console",
        f"--benchmark_out={benchmark_json}",
        "--benchmark_out_format=json",
    ] + benchmark_args

    env = os.environ.copy()
    env.setdefault("LC_ALL", "C")
    result = subprocess.run(cmd, cwd=source_dir, text=True, capture_output=True, env=env)
    console_log.write_text(result.stdout + result.stderr, encoding="utf-8")
    if result.returncode != 0:
        sys.stdout.write(result.stdout)
        sys.stderr.write(result.stderr)
        raise SystemExit(result.returncode)

    manifest = {
        "schema": SCHEMA,
        "timestamp_utc": timestamp,
        "git_commit": git_commit,
        "git_dirty": dirty,
        "project_version": "1.5",
        "benchmark_repetitions": args.repetitions,
        "benchmark_warmup_time": args.warmup_time,
        "benchmark_default_min_time": DEFAULT_MIN_TIME,
        "audio_slice_ms": AUDIO_SLICE_MS,
        "build_type": cmake_cache_value(build_dir, "CMAKE_BUILD_TYPE"),
        "c_compiler": cmake_cache_value(build_dir, "CMAKE_C_COMPILER"),
        "cxx_compiler": cmake_cache_value(build_dir, "CMAKE_CXX_COMPILER"),
        "source_dir": str(source_dir),
        "build_dir": str(build_dir),
        "benchmark_binary": str(binary),
        "python": sys.version.split()[0],
        "platform": platform.platform(),
        "processor": platform.processor(),
        "fixtures": fixtures,
        "command": cmd,
        "benchmark_json": benchmark_json.name,
        "console_log": console_log.name,
    }

    rows = load_benchmark_metrics(benchmark_json)
    metrics_json = out_dir / "spread-metrics.json"
    (out_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    metrics_json.write_text(
        json.dumps(rows, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    write_summary(out_dir / "summary.md", manifest, rows)

    shutil.copy2(benchmark_json, out_dir / "latest-google-benchmark.json")
    print(f"Audio pipeline benchmark report: {out_dir}")


if __name__ == "__main__":
    main()
