#pragma once

#include "dataset.h"
#include "format.h"

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace io_test
{
    class FstreamReader
    {
    public:
        FstreamReader(const std::string &path) : mIn{path, std::ios::binary}
        {
            if (!mIn)
            {
                throw std::runtime_error("failed to open");
            }
            // return values / stream state are not checked, for the sake of simplicity - a production system must check them
            mIn.read(reinterpret_cast<char *>(&mFileHeader), sizeof(mFileHeader));
        }

        // v1: one read of everything into a staging buffer, then split by copying
        Dataset read_version_one()
        {
            Dataset dataset{mFileHeader};

            size_t viewSize = mFileHeader.view_header_size + mFileHeader.payload_size;
            size_t bufferSize = mFileHeader.view_count * viewSize;
            std::vector<char> tmpBuffer(bufferSize);
            mIn.read(tmpBuffer.data(), bufferSize); // one read call

            for (uint32_t i = 0; i < mFileHeader.view_count; ++i)
            {
                const char *p = &tmpBuffer[i * viewSize];
                std::memcpy(&dataset.headers[i], p, sizeof(ViewHeader));
                std::memcpy(dataset.view(i), p + sizeof(ViewHeader), mFileHeader.payload_size);
            }

            return dataset;
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
