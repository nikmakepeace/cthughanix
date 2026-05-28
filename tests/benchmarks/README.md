# Audio Pipeline Benchmarks

These benchmarks use Google Benchmark and focus on the WAV-to-visual-audio path.
They are disabled by default so normal builds do not require benchmark headers.

Configure and build:

```sh
cmake -S . -B build -DCTH_BUILD_BENCHMARKS=ON
cmake --build build --target audio_pipeline_bench
```

Run directly:

```sh
build/tests/benchmarks/audio_pipeline_bench
```

Write a timestamped report:

```sh
cmake --build build --target audio_pipeline_benchmark_report
```

The report runner defaults to:

- 40 benchmark repetitions;
- a timed warmup before measured repetitions (default: 1.0 seconds);
- 10 ms audio slices for chunked ingestion/output pipeline benchmarks;
- millisecond timing units;
- spread metrics over all measured repetitions.

Reports are written under:

```text
build/test-results/audio-pipeline/
```

The report directory contains Google Benchmark JSON, console output,
`manifest.json` with git/build/fixture metadata, `spread-metrics.json`, and
`summary.md`.
