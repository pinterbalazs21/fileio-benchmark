#pragma once

#include "dataset.h"
#include "format.h"

#include <cstdio>
#include <stdexcept>
#include <string>

namespace io_test
{
    class StdioReader
    {
    public:
        StdioReader(const std::string &path, bool buffered = true)
        {
            mFile = std::fopen(path.c_str(), "rb");
            if (!mFile)
            {
                throw std::runtime_error("failed to open");
            }
            if (!buffered) // must happen before the first read
            {
                std::setvbuf(mFile, nullptr, _IONBF, 0);
            }
            // return values / stream state are not checked, for the sake of simplicity - a production system must check them
            std::fread(&mFileHeader, sizeof(mFileHeader), 1, mFile);
        }

        ~StdioReader()
        {
            std::fclose(mFile);
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
