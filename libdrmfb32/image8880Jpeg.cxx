//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2024 Andrew Duncan
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------

#include "image8880Jpeg.h"

//-------------------------------------------------------------------------

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <turbojpeg.h>

//=========================================================================

namespace
{

//-------------------------------------------------------------------------

struct JpegDetails
{
    int m_width{0};
    int m_height{0};
    int m_subsamp{0};
    int m_colourspace{0};
};

//-------------------------------------------------------------------------

class TurboJpegDecode
{
public:

    explicit TurboJpegDecode(std::span<const uint8_t> data);
    ~TurboJpegDecode();

    TurboJpegDecode(const TurboJpegDecode& fb) = delete;
    TurboJpegDecode& operator=(const TurboJpegDecode& fb) = delete;

    TurboJpegDecode(TurboJpegDecode&& fb) = delete;
    TurboJpegDecode& operator=(TurboJpegDecode&& fb) = delete;

    void decode(fb32::Image8880& image);
    JpegDetails details() const noexcept { return m_details; }
    tjhandle instance() const noexcept { return m_instance; }

private:

    std::span<const uint8_t> m_data;
    tjhandle m_instance;
    JpegDetails m_details;
};

//-------------------------------------------------------------------------

TurboJpegDecode::TurboJpegDecode(
    std::span<const uint8_t> data)
:
    m_data{data},
    m_instance{tjInitDecompress()},
    m_details{}
{
    if (m_instance == nullptr)
    {
        throw std::logic_error("Unable to create JPEG Decompressor");
    }

    int result = tjDecompressHeader3(m_instance,
                                     m_data.data(),
                                     m_data.size(),
                                     &m_details.m_width,
                                     &m_details.m_height,
                                     &m_details.m_subsamp,
                                     &m_details.m_colourspace);

    if (result < 0)
    {
        throw std::invalid_argument("Invalid JPEG header");
    }
}

//-------------------------------------------------------------------------

TurboJpegDecode::~TurboJpegDecode()
{
    tjDestroy(m_instance);
}

//-------------------------------------------------------------------------

void
TurboJpegDecode::decode(
    fb32::Image8880& image)
{
    int result = tjDecompress2(m_instance,
                               m_data.data(),
                               m_data.size(),
                               reinterpret_cast<unsigned char*>(image.getBuffer()),
                               m_details.m_width,
                               m_details.m_width * fb32::Interface8880::BytesPerPixel,
                               m_details.m_height,
                               TJPF_BGRX,
                               TJFLAG_ACCURATEDCT);

    if (result < 0)
    {
        throw std::invalid_argument("Unable to decode JPEG");
    }
}

//-------------------------------------------------------------------------

}

//=========================================================================

namespace fb32
{

//-------------------------------------------------------------------------

void
decodeJpeg(
    Image8880& image,
    std::span<const uint8_t> data)
{
    TurboJpegDecode tjd{data};
    tjd.decode(image);
}

//-------------------------------------------------------------------------

Image8880
readJpeg(
    const std::string& name)
{
    const auto length{std::filesystem::file_size(std::filesystem::path(name))};

    std::ifstream ifs{name, std::ios_base::binary};
    std::vector<uint8_t> buffer(length);
    ifs.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    TurboJpegDecode tjd{buffer};
    auto details{tjd.details()};
    fb32::Image8880 image{details.m_width, details.m_height};

    tjd.decode(image);

    return image;
}

//-------------------------------------------------------------------------

}

