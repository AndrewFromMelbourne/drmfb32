
//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2023 Andrew Duncan
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

#include "image8880Qoi.h"

//-------------------------------------------------------------------------

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

//=========================================================================

namespace
{

//-------------------------------------------------------------------------

constexpr int QOI_HEADER_SIZE{14};
constexpr int QOI_FOOTER_SIZE{8};
constexpr uint32_t QOI_MAGIC{('q' << 24) | ('o' << 16) | ('i' << 8) | 'f'};

constexpr uint8_t QOI_OP_RGB{0xFE};
constexpr uint8_t QOI_OP_RGBA{0xFF};

constexpr uint8_t QOI_MASK_OP{0xC0};
constexpr uint8_t QOI_UNMASK{0x3F};

constexpr uint8_t QOI_MASKED_OP_INDEX{0x00};
constexpr uint8_t QOI_MASKED_OP_DIFF{0x40};
constexpr uint8_t QOI_MASKED_OP_LUMA{0x80};
constexpr uint8_t QOI_MASKED_OP_RUN{0xC0};

//-------------------------------------------------------------------------

class QoiHeader
{
public:

    QoiHeader(const std::array<uint8_t, QOI_HEADER_SIZE>& data);

    uint32_t getWidth() const noexcept { return m_width; }
    uint32_t getHeight() const noexcept { return m_height; }
    uint8_t getChannels() const noexcept { return m_channels; }
    uint8_t getColorSpace() const noexcept { return m_colorSpace; }

private:

    uint32_t m_width;
    uint32_t m_height;
    uint8_t m_channels;
    uint8_t m_colorSpace;
};

//-------------------------------------------------------------------------

QoiHeader::QoiHeader(
    const std::array<uint8_t, QOI_HEADER_SIZE>& data)
:
    m_width{0},
    m_height{0},
    m_channels{0},
    m_colorSpace{0}
{
    uint32_t magic =
        (data[0] << 24) |
        (data[1] << 16) |
        (data[2] << 8) |
        data[3];

    if (magic != QOI_MAGIC)
    {
        throw std::invalid_argument("QOI bad magic value");
    }

    m_width =
        (data[4] << 24) |
        (data[5] << 16) |
        (data[6] << 8) |
        data[7];
    m_height =
        (data[8] << 24) |
        (data[9] << 16) |
        (data[10] << 8) |
        data[11];

    if ((m_width == 0) or (m_height == 0))
    {
        throw std::invalid_argument("QOI width or height is zero");
    }

    m_channels = data[12];

    if ((m_channels < 3) or (m_channels > 4))
    {
        throw std::invalid_argument("QOI channels must be either 3 or 4");
    }

    m_colorSpace = data[13];

    if (m_colorSpace > 1)
    {
        throw std::invalid_argument("QOI color space must be either 0 or 1");
    }
}

//-------------------------------------------------------------------------

void
checkFooter(
    const std::array<uint8_t, QOI_FOOTER_SIZE>& data)
{
    const std::array<uint8_t, QOI_FOOTER_SIZE> expected{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    };

    if (data != expected)
    {
        throw std::invalid_argument("QOI bad footer value");
    }
}

//-------------------------------------------------------------------------

struct QoiRGBA
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

//-------------------------------------------------------------------------

int
rgbaHashQoi(
    const QoiRGBA& rgba) noexcept
{
    return (rgba.r * 3 + rgba.g * 5 + rgba.b * 7 + rgba.a * 11) % 64;
}

//-------------------------------------------------------------------------

fb32::Image8880
decodeQoi(
    const QoiHeader& header,
    const std::vector<uint8_t>& data,
    const fb32::RGB8880& background)
{

    fb32::Image8880 image(header.getWidth(), header.getHeight());

    QoiRGBA currentRGBA{ .r = 0, .g = 0, .b = 0, .a = 255 };

    std::array<QoiRGBA, 64> hashTableRGBA;

    for (auto& rgba : hashTableRGBA)
    {
        rgba = QoiRGBA{ .r = 0, .g = 0, .b = 0, .a = 0 };
    }

    const auto pixels{header.getWidth() * header.getHeight()};
    auto d{data.begin()};
    int run{};

    for (int i = 0 ; (i < pixels) and (d != data.end()) ; ++i)
    {
        if (run)
        {
            --run;
        }
        else
        {
            const auto value = *d++;

            if (value == QOI_OP_RGB)
            {
                currentRGBA.r = *d++;
                currentRGBA.g = *d++;
                currentRGBA.b = *d++;

                hashTableRGBA[rgbaHashQoi(currentRGBA)] = currentRGBA;
            }
            else if (value == QOI_OP_RGBA)
            {
                currentRGBA.r = *d++;
                currentRGBA.g = *d++;
                currentRGBA.b = *d++;
                currentRGBA.a = *d++;

                hashTableRGBA[rgbaHashQoi(currentRGBA)] = currentRGBA;
            }
            else
            {
                switch (value & QOI_MASK_OP)
                {
                    case QOI_MASKED_OP_INDEX:

                        currentRGBA = hashTableRGBA[value];

                        break;

                    case QOI_MASKED_OP_DIFF:

                        currentRGBA.r += ((value >> 4) & 0x03) - 2;
                        currentRGBA.g += ((value >> 2) & 0x03) - 2;
                        currentRGBA.b += (value & 0x03) - 2;

                        hashTableRGBA[rgbaHashQoi(currentRGBA)] = currentRGBA;

                        break;

                    case QOI_MASKED_OP_LUMA:
                    {
                        const auto diffs{*d++};
                        const auto dg{(value & 0x3f) - 32};
                        const auto dr_dg{(diffs >> 4) & 0x0F};
                        const auto db_dg{diffs & 0x0F};

                        currentRGBA.r += dg - 8 + dr_dg;
                        currentRGBA.g += dg;
                        currentRGBA.b += dg - 8 + db_dg;

                        hashTableRGBA[rgbaHashQoi(currentRGBA)] = currentRGBA;

                        break;
                    }
                    case QOI_MASKED_OP_RUN:

                        run = value & QOI_UNMASK;

                        break;
                }
            }
        }

        const int x = i % header.getWidth();
        const int y = i / header.getWidth();
        const fb32::RGB8880 rgb{currentRGBA.r, currentRGBA.g, currentRGBA.b};

        image.setPixelRGB(fb32::Interface8880Point{x, y},
                          rgb.blend(currentRGBA.a, background));
    }

    return image;
}

//-------------------------------------------------------------------------

}

//=========================================================================

namespace fb32
{

//-------------------------------------------------------------------------

Image8880
readQoi(
    const std::string& name,
    const fb32::RGB8880& background)
{
    const auto length{std::filesystem::file_size(std::filesystem::path(name))};

    std::ifstream ifs{name, std::ios_base::binary};

    std::array<uint8_t, QOI_HEADER_SIZE> rawHeader{};
    ifs.read(reinterpret_cast<char*>(rawHeader.data()), rawHeader.size());
    const QoiHeader header(rawHeader);

    std::vector<uint8_t> buffer(length - QOI_HEADER_SIZE - QOI_FOOTER_SIZE);
    ifs.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    std::array<uint8_t, QOI_FOOTER_SIZE> rawFooter{};
    ifs.read(reinterpret_cast<char*>(rawFooter.data()), rawFooter.size());
    checkFooter(rawFooter);

    return decodeQoi(header, buffer, background);
}

//-------------------------------------------------------------------------

}
