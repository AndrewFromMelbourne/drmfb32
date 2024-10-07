
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

#include <array>
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

fb32::Image8880
decodeJpeg(
    const std::vector<uint8_t>& data)
{
    tjhandle tjInstance = tjInitDecompress();

    if (tjInstance == nullptr)
    {
        tjDestroy(tjInstance);
        throw std::logic_error("Unable to create JPEG Decompressor");
    }

    int width{};
    int height{};
    int subsamp{};
    int colourspace{};
    int result{};

    result = tjDecompressHeader3(tjInstance,
                                 data.data(),
                                 data.size(),
                                 &width,
                                 &height,
                                 &subsamp,
                                 &colourspace);

    if (result < 0)
    {
        tjDestroy(tjInstance);
        throw std::invalid_argument("Invalid JPEG header");
    }

    fb32::Image8880 image(width, height);

    result = tjDecompress2(tjInstance,
                           data.data(),
                           data.size(),
                           reinterpret_cast<unsigned char*>(image.getBuffer()),
                           width,
                           width * 4,
                           height,
                           TJPF_BGRX,
                           TJFLAG_ACCURATEDCT);

    if (result < 0)
    {
        tjDestroy(tjInstance);
        throw std::invalid_argument("Unable to decode JPEG");
    }

    tjDestroy(tjInstance);

    return image;
}

//-------------------------------------------------------------------------

}

//=========================================================================

namespace fb32
{

//-------------------------------------------------------------------------

Image8880
readJpeg(
    const std::string& name)
{
    const auto length{std::filesystem::file_size(std::filesystem::path(name))};

    std::ifstream ifs{name, std::ios_base::binary};
    std::vector<uint8_t> buffer(length);
    ifs.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    return decodeJpeg(buffer);
}

//-------------------------------------------------------------------------

}
