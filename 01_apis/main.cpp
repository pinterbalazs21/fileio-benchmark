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

    bench.measure("fstream", "one_syscall", [&]
                  { return FstreamReader{path}.read_version_one(); });
    bench.measure("fstream", "continuous_many", [&]
                  { return FstreamReader{path}.read_version_two(); });
    bench.measure("fstream", "seek_many", [&]
                  { return FstreamReader{path}.read_version_three(); });
    bench.measure("stdio", "one_syscall", [&]
                  { return StdioReader{path}.read_version_one(); });
    bench.measure("stdio", "continuous_many", [&]
                  { return StdioReader{path}.read_version_two(); });
    bench.measure("stdio", "seek_many", [&]
                  { return StdioReader{path}.read_version_three(); });
#if __has_include(<unistd.h>)
    bench.measure("posix", "one_syscall", [&]
                  { return PosixReader{path}.read_version_one(); });
    bench.measure("posix", "continuous_many", [&]
                  { return PosixReader{path}.read_version_two(); });
    bench.measure("posix", "seek_many", [&]
                  { return PosixReader{path}.read_version_three(); });
#endif
    return 0;
}
