#pragma once

#include "format.h"

#include <cstddef>
#include <vector>

namespace io_test
{

    struct Dataset
    {
        FileHeader fileHeader;
        std::vector<ViewHeader> headers; // view_count entries
        std::vector<Pixel> pixels;       // view_count * width * height, views back to back

        Dataset(FileHeader fileHeader) : fileHeader{fileHeader}
        {
            headers.resize(fileHeader.view_count);
            pixels.resize(fileHeader.view_count * pixelsPerView());
        }

        size_t pixelsPerView() const { return size_t{fileHeader.width} * fileHeader.height; }
        Pixel *view(uint32_t i) { return pixels.data() + i * pixelsPerView(); } // first pixel of view i
    };

}

// mock computational library that consumes (and perhaps defines) the Dataset
namespace calc_lib
{
    using CalculationResult = void;
    inline CalculationResult doCalculation([[maybe_unused]] const io_test::Dataset &dataset)
    {
        // do computationally heavy calculation, data is expected in dataset format
    }
}
