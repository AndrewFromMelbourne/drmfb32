//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2022 Andrew Duncan
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

#include <stdexcept>

#include "image8880.h"

//-------------------------------------------------------------------------

using size_type = std::vector<uint32_t>::size_type;

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    int width,
    int height,
    uint8_t numberOfFrames)
:
    m_width{width},
    m_height{height},
    m_frame{0},
    m_numberOfFrames{numberOfFrames},
    m_buffer(width * height * numberOfFrames)
{
}

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    int width,
    int height,
    const std::vector<uint32_t>& buffer,
    uint8_t numberOfFrames)
:
    m_width{width},
    m_height{height},
    m_frame{0},
    m_numberOfFrames{numberOfFrames},
    m_buffer(buffer)
{
    size_t minBufferSize = width * height * numberOfFrames;

    if (m_buffer.size() < minBufferSize)
    {
        m_buffer.resize(minBufferSize);
    }
}

//-------------------------------------------------------------------------

void
fb32::Image8880::setFrame(
    uint8_t frame)
{
    if (frame < m_numberOfFrames)
    {
        m_frame = frame;
    }
}

//-------------------------------------------------------------------------

void
fb32::Image8880::clear(
    uint32_t rgb)
{
    std::fill(m_buffer.begin(), m_buffer.end(), rgb);
}

//-------------------------------------------------------------------------

bool
fb32::Image8880::setPixel(
    const Interface8880Point& p,
    uint32_t rgb,
    uint8_t frame)
{
    bool isValid{validPixel(p)};

    if (isValid)
    {
        m_buffer[offset(p, frame)] = rgb;
    }

    return isValid;
}

//-------------------------------------------------------------------------

std::optional<fb32::RGB8880>
fb32::Image8880::getPixelRGB(
    const Interface8880Point& p,
    uint8_t frame) const
{
    if (not validPixel(p))
    {
        return {};
    }

    return RGB8880(m_buffer[offset(p, frame)]);
}

//-------------------------------------------------------------------------

std::optional<uint32_t>
fb32::Image8880::getPixel(
    const Interface8880Point& p,
    uint8_t frame) const
{
    if (not validPixel(p))
    {
        return {};
    }

    return m_buffer[offset(p, frame)];
}

//-------------------------------------------------------------------------

const uint32_t*
fb32::Image8880::getRow(
    int y) const
{
    const Interface8880Point p{0, y};

    if (validPixel(p))
    {
        return  m_buffer.data() + offset(p);
    }
    else
    {
        return nullptr;
    }
}


//-------------------------------------------------------------------------

fb32::Image8880
fb32::Image8880::resizeNearestNeighbour(
    int width,
    int height) const
{
    if ((width <= 0) or (height <= 0))
    {
        throw std::invalid_argument("width and height must be greater than zero");
    }

    const int a = (width > m_width) ? 0 : 1;
    const int b = (height > m_height) ? 0 : 1;

    Image8880 image{width, height, m_numberOfFrames};

    for (uint8_t frame = 0 ; frame < m_numberOfFrames ; ++frame)
    {
        for (int j = 0 ; j < height ; ++j)
        {
            const int y = (j * (m_height - b)) / (height - b);
            for (int i = 0 ; i < width ; ++i)
            {
                const int x = (i * (m_width - a)) / (width - a);
                auto pixel{getPixel(Interface8880Point{x, y}, frame)};

                if (pixel.has_value())
                {
                    image.setPixel(Interface8880Point{i, j}, pixel.value(), frame);
                }
            }
        }
    }

    return image;
}

//-------------------------------------------------------------------------

size_t
fb32::Image8880::offset(
    const Interface8880Point& p,
    uint8_t frame) const
{
    return p.x() + (p.y() * m_width) + (m_width * m_height * frame);
}

