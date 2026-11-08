#include "bench.h"
#include "fstreamReader.h"
#include "stdioReader.h"

#if __has_include(<unistd.h>)
#include "posixReader.h"
#endif

using namespace io_test;

int main(int argc, char *argv[])
{
    std::string path;
    Bench bench;
    if (!parseArgs(argc, argv, path, bench))
    {
        return -1;
    }
    if (bench.filter.empty())
    {
        printMetadata(path, FstreamReader{path}.header());
    }

    bench.measure("fstream_nobuf", "continuous_many", [&]
                  { return FstreamReader{path, false}.read_version_two(); });
    bench.measure("fstream_nobuf", "seek_many", [&]
                  { return FstreamReader{path, false}.read_version_three(); });
    bench.measure("fstream_buffered", "continuous_many", [&]
                  { return FstreamReader{path, true}.read_version_two(); });
    bench.measure("fstream_buffered", "seek_many", [&]
                  { return FstreamReader{path, true}.read_version_three(); });
    bench.measure("stdio_nobuf", "continuous_many", [&]
                  { return StdioReader{path, false}.read_version_two(); });
    bench.measure("stdio_nobuf", "seek_many", [&]
                  { return StdioReader{path, false}.read_version_three(); });
    bench.measure("stdio_buffered", "continuous_many", [&]
                  { return StdioReader{path, true}.read_version_two(); });
    bench.measure("stdio_buffered", "seek_many", [&]
                  { return StdioReader{path, true}.read_version_three(); });
#if __has_include(<unistd.h>)
    bench.measure("posix", "continuous_many", [&]
                  { return PosixReader{path}.read_version_two(); });
    bench.measure("posix", "seek_many", [&]
                  { return PosixReader{path}.read_version_three(); });
#endif
    return 0;
}
