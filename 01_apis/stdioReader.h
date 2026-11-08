#pragma once

#include "dataset.h"
#include "format.h"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace io_test
{
    class StdioReader
    {
    public:
        StdioReader(const std::string &path)
        {
            mFile = std::fopen(path.c_str(), "rb");
            if (!mFile)
            {
                throw std::runtime_error("failed to open");
            }
            // return values / stream state are not checked, for the sake of simplicity - a production system must check them
            std::fread(&mFileHeader, sizeof(mFileHeader), 1, mFile);
        }

        ~StdioReader()
        {
            std::fclose(mFile);
        }

        // v1: one read of everything into a staging buffer, then split by copying
        Dataset read_version_one()
        {
            Dataset dataset{mFileHeader};

            size_t viewSize = mFileHeader.view_header_size + mFileHeader.payload_size;
            size_t bufferSize = mFileHeader.view_count * viewSize;
            std::vector<char> tmpBuffer(bufferSize);
            std::fread(tmpBuffer.data(), bufferSize, 1, mFile); // one read call

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
                std::fread(&dataset.headers[i], sizeof(ViewHeader), 1, mFile);
                std::fread(dataset.view(i), mFileHeader.payload_size, 1, mFile);
            }

            return dataset;
        }

        // v3: all headers first, then all payloads, directly into the Dataset
        Dataset read_version_three()
        {
            Dataset dataset{mFileHeader};

            for (uint32_t i = 0; i < mFileHeader.view_count; ++i)
            {
                std::fread(&dataset.headers[i], sizeof(ViewHeader), 1, mFile);
                std::fseek(mFile, mFileHeader.payload_size, SEEK_CUR);
            }

            std::fseek(mFile, mFileHeader.file_header_size, SEEK_SET); // back to the first view
            for (uint32_t i = 0; i < mFileHeader.view_count; ++i)
            {
                std::fseek(mFile, sizeof(ViewHeader), SEEK_CUR);
                std::fread(dataset.view(i), mFileHeader.payload_size, 1, mFile);
            }

            return dataset;
        }

        FileHeader header() { return mFileHeader; }

    private:
        std::FILE *mFile = nullptr;
        FileHeader mFileHeader{};
    };
}
