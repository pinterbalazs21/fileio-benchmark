#pragma once

#include "dataset.h"
#include "format.h"

#include <fstream>
#include <stdexcept>
#include <string>

namespace io_test
{
    class FstreamReader
    {
    public:
        FstreamReader(const std::string &path, bool buffered = true)
        {
            // libstdc++ / libc++ only accept a buffer change before open, MSVC only after
#ifndef _MSC_VER
            if (!buffered)
            {
                mIn.rdbuf()->pubsetbuf(nullptr, 0);
            }
#endif
            mIn.open(path, std::ios::binary);
            if (!mIn)
            {
                throw std::runtime_error("failed to open");
            }
#ifdef _MSC_VER
            if (!buffered)
            {
                mIn.rdbuf()->pubsetbuf(nullptr, 0);
            }
#endif
            // return values / stream state are not checked, for the sake of simplicity - a production system must check them
            mIn.read(reinterpret_cast<char *>(&mFileHeader), sizeof(mFileHeader));
        }

        // v2: header, payload, header, payload, ... directly into the Dataset
        Dataset read_version_two()
        {
            Dataset dataset{mFileHeader};

            for (uint32_t i = 0; i < mFileHeader.view_count; ++i)
            {
                mIn.read(reinterpret_cast<char *>(&dataset.headers[i]), sizeof(ViewHeader));
                mIn.read(reinterpret_cast<char *>(dataset.view(i)), mFileHeader.payload_size);
            }

            return dataset;
        }

        // v3: all headers first, then all payloads, directly into the Dataset
        Dataset read_version_three()
        {
            Dataset dataset{mFileHeader};

            for (uint32_t i = 0; i < mFileHeader.view_count; ++i)
            {
                mIn.read(reinterpret_cast<char *>(&dataset.headers[i]), sizeof(ViewHeader));
                mIn.seekg(mFileHeader.payload_size, std::ios::cur);
            }

            mIn.seekg(mFileHeader.file_header_size, std::ios::beg); // back to the first view
            for (uint32_t i = 0; i < mFileHeader.view_count; ++i)
            {
                mIn.seekg(sizeof(ViewHeader), std::ios::cur);
                mIn.read(reinterpret_cast<char *>(dataset.view(i)), mFileHeader.payload_size);
            }

            return dataset;
        }

        FileHeader header() { return mFileHeader; }

    private:
        std::ifstream mIn;
        FileHeader mFileHeader{};
    };
}
