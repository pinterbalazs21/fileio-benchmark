# File I/O benchmark

How fast can C++ load a raw image stream (a file header, then a header + payload per view) into a
structure of arrays? `std::ifstream` vs `FILE*` vs POSIX, three read patterns, and buffering.

Medium article: [The Fastest Way to Read a Raw Image Stream](https://medium.com/@pinterbalazs21/the-fastest-way-to-read-a-raw-image-stream-d06e58542233)

## Build

Needs CMake 3.20+ and a C++23 compiler (tested with MSVC 14.51 and GCC 15.2). Run from this folder:

```
cmake --preset default
cmake --build --preset release
```

Binaries end up in `build/Release/` (Visual Studio) or `build/` (Linux).

## Usage

Generate test data: `generate_test_data <file> <views> <width> <height>`, e.g. 2000 views of 896 x 64
pixels (~229 MB):

```
generate_test_data data.bin 2000 896 64
```

Measure: `bench_0X <file> <runs> [api[,version]]`. The first load of each variant is checked against the
generated data before any time is reported. Output is CSV: `api,version,run,time_ms`.

```
bench_01 data.bin 10                    # all variants, 10 runs each
bench_02 data.bin 10 stdio_buffered     # only one API
```

For the article, every load ran in a fresh process (`runs` = 1, repeated 10 times), with the file
already in the page cache. On WSL, keep the test file on the Linux filesystem, not under `/mnt/c`.

## Layout

- `common/`: file format, `Dataset`, test data generator, shared benchmark harness (`bench.h`)
- `01_apis/`: `std::ifstream` vs `FILE*` vs POSIX, three read patterns (`bench_01`)
- `02_buffered/`: the same readers with and without a user-space buffer (`bench_02`)

POSIX is only built where `<unistd.h>` exists (Linux, macOS).

## License

MIT, see [LICENSE](LICENSE).
