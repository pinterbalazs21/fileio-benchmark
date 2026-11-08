#pragma once

#include "dataset.h"
#include "format.h"

#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace io_test
{
    class PosixReader // no user space buffer at all: every read is a system call
    {
    public:
        PosixReader(const std::string &path)
        {
            mFd = open(path.c_str(), O_RDONLY);
            if (mFd < 0)
            {
                throw std::runtime_error("failed to open");
            }
            // return values / stream state are not checked, for the sake of simplicity - a production system must check them
            read(mFd, &mFileHeader, sizeof(mFileHeader));
        }

        ~PosixReader()
        {
            close(mFd);
        }

        // v2: header, payload, header, payload, ... directly into the Dataset
        Dataset read_version_two()
        {
            Dataset dataset{mFileHeader};

            for (uint32_t i = 0; i < mFileHeader.view_count; ++i)
            {
                read(mFd, &dataset.headers[i], sizeof(ViewHeader));
                read(mFd, dataset.view(i), mFileHeader.payload_size);
            }

            return dataset;
        }

        // v3: all headers first, then all payloads, directly into the Dataset
        Dataset read_version_three()
        {
            Dataset dataset{mFileHeader};

            for (uint32_t i = 0; i < mFileHeader.view_count; ++i)
            {
                read(mFd, &dataset.headers[i], sizeof(ViewHeader));
                lseek(mFd, mFileHeader.payload_size, SEEK_CUR);
            }

            lseek(mFd, mFileHeader.file_header_size, SEEK_SET); // back to the first view
            for (uint32_t i = 0; i < mFileHeader.view_count; ++i)
            {
                lseek(mFd, sizeof(ViewHeader), SEEK_CUR);
                read(mFd, dataset.view(i), mFileHeader.payload_size);
            }

            return dataset;
        }

        FileHeader header() { return mFileHeader; }

    private:
        int mFd = -1;
        FileHeader mFileHeader{};
    };
}
