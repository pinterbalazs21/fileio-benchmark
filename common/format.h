#pragma once

#include <cstdint>
#include <type_traits>

namespace io_test
{

    using Pixel = uint16_t;
    constexpr uint32_t kVersion = 1;

    // Minimal headers for the demo, a real format carries much more.
    // Layout rule for both headers: data first, padding last, no gap in between.
    // Every field sits at an offset that is a multiple of its own size, so the compiler never inserts hidden padding.
    // That's why the only 64-bit field (ViewHeader::timestamp_ns) comes first.

    struct FileHeader
    {
        uint32_t version;          // kVersion
        uint32_t file_header_size; // sizeof(FileHeader)
        uint32_t view_header_size; // sizeof(ViewHeader)
        uint32_t payload_size;     // bytes per view = width * height * bytes_per_pixel
        uint32_t view_count;
        uint32_t width;
        uint32_t height;
        uint32_t bytes_per_pixel;
        uint8_t reserved[32];
    };
    static_assert(sizeof(FileHeader) == 64); // one cache line
    static_assert(std::has_unique_object_representations_v<FileHeader>); // no hidden padding bytes

    struct ViewHeader
    {
        uint64_t timestamp_ns;
        uint32_t index;
        float angle_deg;
        int32_t table_pos_um;
        uint32_t payload_size; // repeated so a view is self-describing
        uint8_t reserved[8];
    };
    static_assert(sizeof(ViewHeader) == 32); // two per cache line

}
