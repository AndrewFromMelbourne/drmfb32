//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2026 Andrew Duncan
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

#include "image8880Webp.h"

//-------------------------------------------------------------------------

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include <webp/decode.h>

//=========================================================================

namespace
{

//-------------------------------------------------------------------------

struct WebpDetails
{
    int m_width{0};
    int m_height{0};
};

//-------------------------------------------------------------------------

class WebpDecode
{
public:

    explicit WebpDecode(std::span<const uint8_t> data);
    ~WebpDecode();

    WebpDecode(const WebpDecode& fb) = delete;
    WebpDecode& operator=(const WebpDecode& fb) = delete;

    WebpDecode(WebpDecode&& fb) = delete;
    WebpDecode& operator=(WebpDecode&& fb) = delete;

    void decode(fb32::Image8880& image);
    void decodeToGrey(fb32::Image8880& image);
    WebpDetails details() const noexcept { return m_details; }

private:

    std::span<const uint8_t> m_data;
    WebpDetails m_details;
};

//-------------------------------------------------------------------------

WebpDecode::WebpDecode(
    std::span<const uint8_t> data)
:
    m_data{data},
    m_details{}
{
    int result = WebPGetInfo(m_data.data(),
                             m_data.size(),
                             &m_details.m_width,
                             &m_details.m_height);

    if (result < 0)
    {
        throw std::invalid_argument("Invalid WEBP header");
    }
}

//-------------------------------------------------------------------------

WebpDecode::~WebpDecode()
{
}

//-------------------------------------------------------------------------

void
WebpDecode::decode(
    fb32::Image8880& image)
{
    auto* buffer = WebPDecodeRGB(m_data.data(),
                                 m_data.size(),
                                 &m_details.m_width,
                                 &m_details.m_height);
    if (not buffer)
    {
        throw std::invalid_argument("Unable to decode WEBP");
    }

    const auto bufferSize{static_cast<std::size_t>(m_details.m_width * m_details.m_height * 3)};
    std::span<const uint8_t> rgbBuffer{buffer, bufferSize};

    auto imageIter = begin(image.getBuffer());
    auto rgbIter = begin(rgbBuffer);

    for (auto i = 0; i < image.getDimensions().area(); ++i)
    {
        const auto r = *(rgbIter++);
        const auto g = *(rgbIter++);
        const auto b = *(rgbIter++);
        const fb32::RGB8880 rgb{r, g, b};
        *imageIter++ = rgb.get8880();
    }

    WebPFree(buffer);
}

//-------------------------------------------------------------------------

}

//=========================================================================

namespace fb32
{

//-------------------------------------------------------------------------

void
decodeWebp(
    fb32::Image8880& image,
    std::span<const uint8_t> data)
{
    WebpDecode wd{data};
    wd.decode(image);
}

//-------------------------------------------------------------------------

fb32::Image8880
readWebp(
    const std::string& name)
{
    const auto length{std::filesystem::file_size(std::filesystem::path(name))};

    std::ifstream ifs{name, std::ios_base::binary};
    std::vector<uint8_t> buffer(length);
    ifs.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    WebpDecode wd{buffer};
    auto details{wd.details()};
    const fb32::Dimensions8880 d{details.m_width, details.m_height};
    fb32::Image8880 image{d};

    wd.decode(image);

    return image;
}

//-------------------------------------------------------------------------

}

