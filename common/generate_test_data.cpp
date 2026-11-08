#include "format.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace io_test;

namespace
{

    ViewHeader makeViewHeader(uint32_t index, uint32_t width, uint32_t height)
    {
        // only index and timestamp are checked by the benchmark, the rest mimics a real header
        return ViewHeader{
            .timestamp_ns = uint64_t{index} * 500'000,             // 2000 views per second
            .index = index,
            .angle_deg = static_cast<float>(index % 1000) * 0.36f, // 1000 views per rotation
            .table_pos_um = static_cast<int32_t>(index * 40),      // couch moves 40 um per view
            .payload_size = static_cast<uint32_t>(width * height * sizeof(Pixel)),
            .reserved = {},
        };
    }

}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        std::cerr << "usage: " << argv[0] << " <file> <views> <width> <height>\n";
        return -1;
    }
    const char *path = argv[1];
    const auto viewCount = static_cast<uint32_t>(std::stoi(argv[2]));
    const auto width = static_cast<uint32_t>(std::stoi(argv[3]));
    const auto height = static_cast<uint32_t>(std::stoi(argv[4]));

    const uint32_t payloadSize = width * height * sizeof(Pixel);

    const FileHeader fileHeader{
        .version = kVersion,
        .file_header_size = sizeof(FileHeader),
        .view_header_size = sizeof(ViewHeader),
        .payload_size = payloadSize,
        .view_count = viewCount,
        .width = width,
        .height = height,
        .bytes_per_pixel = sizeof(Pixel),
        .reserved = {},
    };

    std::ofstream out(path, std::ios::binary);
    if (!out)
    {
        std::cerr << "open failed\n";
        return 1;
    }

    out.write(reinterpret_cast<const char *>(&fileHeader), sizeof(fileHeader));

    std::vector<Pixel> pixels(width * height);
    for (uint32_t i = 0; i < viewCount; ++i)
    {
        for (uint32_t pixel = 0; pixel < width * height; ++pixel)
        {
            // deterministic pattern, so the benchmark can check every pixel; wraps around at 65536, same as verify()
            pixels[pixel] = static_cast<Pixel>(i * 100 + pixel);
        }
        const ViewHeader viewHeader = makeViewHeader(i, width, height);
        out.write(reinterpret_cast<const char *>(&viewHeader), sizeof(viewHeader));
        out.write(reinterpret_cast<const char *>(pixels.data()), sizeof(Pixel) * pixels.size());
    }

    out.close();
    if (!out)
    {
        std::cerr << "write failed\n";
        return 1;
    }

    std::cout << "wrote " << sizeof(FileHeader) + viewCount * (sizeof(ViewHeader) + payloadSize) << " bytes\n";
}
