#pragma once

#include "dataset.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

namespace io_test
{
    // checks the pattern written by generate_test_data, only on the first run of each measurement
    inline bool verify(Dataset &dataset)
    {
        for (uint32_t i = 0; i < dataset.fileHeader.view_count; ++i)
        {
            if (dataset.headers[i].index != i || dataset.headers[i].timestamp_ns != i * 500'000ull)
            {
                return false;
            }
            const Pixel *view = dataset.view(i);
            for (size_t pixel = 0; pixel < dataset.pixelsPerView(); ++pixel)
            {
                if (view[pixel] != static_cast<Pixel>(i * 100 + pixel)) // wraps around at 65536, like the generator
                {
                    return false;
                }
            }
        }
        return true;
    }

    inline void printMetadata(const std::string &path, const FileHeader &fileHeader)
    {
#ifdef _WIN32
        const std::string platform = "windows";
#else
        const std::string platform = "linux";
#endif
        std::cout << "views=" << fileHeader.view_count << ' ' << fileHeader.width << 'x' << fileHeader.height
                  << " size=" << std::filesystem::file_size(path) << ' ' << platform << '\n';
    }

    struct Bench
    {
        int runs = 1;
        std::string filter;

        bool selected(const std::string &api, const std::string &version) const
        {
            return filter.empty() || filter == api || filter == api + "," + version;
        }

        template <typename Load>
        void measure(const std::string &api, const std::string &version, Load load) const
        {
            if (!selected(api, version))
            {
                return;
            }
            for (int run = 0; run < runs; ++run)
            {
                const auto start = std::chrono::steady_clock::now();
                Dataset dataset = load();
                const auto end = std::chrono::steady_clock::now();
                calc_lib::doCalculation(dataset); // stands in for the heavy calculation library, a no-op here and not timed

                if (run == 0 && !verify(dataset))
                {
                    std::cerr << "verify failed: " << api << ' ' << version << '\n';
                    return;
                }

                const double ms = std::chrono::duration<double, std::milli>(end - start).count();
                std::cout << api << ',' << version << ',' << run << ',' << std::fixed << std::setprecision(3) << ms << '\n';
            }
        }
    };

    inline bool parseArgs(int argc, char *argv[], std::string &path, Bench &bench)
    {
        if (argc != 3 && argc != 4)
        {
            std::cerr << "usage: " << argv[0] << " <file> <runs> [api[,version]]\n";
            return false;
        }
        path = argv[1];
        bench.runs = std::stoi(argv[2]);
        bench.filter = argc == 4 ? argv[3] : "";
        return true;
    }
}
