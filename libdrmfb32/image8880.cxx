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

#include <algorithm>
#include <cmath>
#include <functional>
#include <numbers>
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
fb32::Image8880::resizeBilinearInterpolation(
    int width,
    int height) const
{
    if ((width <= 0) or (height <= 0))
    {
        throw std::invalid_argument("width and height must be greater than zero");
    }

    Image8880 image{width, height, m_numberOfFrames};
    resizeToBilinearInterpolation(image);

    return image;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::Image8880::resizeLanczos3Interpolation(
    int width,
    int height) const
{
    if ((width <= 0) or (height <= 0))
    {
        throw std::invalid_argument("width and height must be greater than zero");
    }

    Image8880 image{width, height, m_numberOfFrames};
    resizeToLanczos3Interpolation(image);

    return image;
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

    Image8880 image{width, height, m_numberOfFrames};
    resizeToNearestNeighbour(image);

    return image;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::Image8880::resizeToBilinearInterpolation(
    fb32::Image8880& image) const
{
    const auto xScale = (image.getWidth() > 1)
                      ? (m_width - 1.0f) / (image.getWidth() - 1.0f)
                      : 0.0f;
    const auto yScale = (image.getHeight() > 1)
                      ? (m_height - 1.0f) / (image.getHeight() - 1.0f)
                      : 0.0f;

    for (uint8_t frame = 0 ; frame < m_numberOfFrames ; ++frame)
    {
        for (int j = 0; j < image.getHeight(); ++j)
        {
            for (int i = 0; i < image.getWidth(); ++i)
            {
                int xLow = static_cast<int>(std::floor(xScale * i));
                int yLow = static_cast<int>(std::floor(yScale * j));
                int xHigh = static_cast<int>(std::ceil(xScale * i));
                int yHigh = static_cast<int>(std::ceil(yScale * j));

                const auto xWeight = (xScale * i) - xLow;
                const auto yWeight = (yScale * j) - yLow;

                auto a = *getPixelRGB(Interface8880Point{xLow, yLow});
                auto b = *getPixelRGB(Interface8880Point{xHigh, yLow});
                auto c = *getPixelRGB(Interface8880Point{xLow, yHigh});
                auto d = *getPixelRGB(Interface8880Point{xHigh, yHigh});

                const auto aWeight = (1.0f - xWeight) * (1.0f - yWeight);
                const auto bWeight = xWeight * (1.0f - yWeight);
                const auto cWeight = (1.0f - xWeight) * yWeight;
                const auto dWeight = xWeight * yWeight;

                typedef  uint8_t (RGB8880::*RGB8880MemFn)() const;

                auto evaluate = [&](RGB8880MemFn get) -> uint8_t
                {
                    float value = std::invoke(get, a) * aWeight +
                                  std::invoke(get, b) * bWeight +
                                  std::invoke(get, c) * cWeight +
                                  std::invoke(get, d) * dWeight;

                    return static_cast<uint8_t>(std::clamp(value, 0.0f, 255.0f));
                };

                RGB8880 rgb{evaluate(&RGB8880::getRed),
                            evaluate(&RGB8880::getGreen),
                            evaluate(&RGB8880::getBlue)};

                image.setPixelRGB(Interface8880Point{i, j}, rgb, frame);
            }
        }
    }

    return image;
}

//-------------------------------------------------------------------------
fb32::Image8880&
fb32::Image8880::resizeToLanczos3Interpolation(
    fb32::Image8880& image) const
{
    constexpr int a{3};

    auto kernel = [](float x, int a) -> float
    {
        const auto pi = std::numbers::pi_v<float>;

        if (x == 0.0f)
        {
            return 1.0f;
        }

        if (x < -a or x > a)
        {
            return 0.0f;
        }

        return (a * std::sin(pi * x) * std::sin(pi * x / a)) /
               (pi * pi * x * x);
    };

    const auto xScale = (image.getWidth() > 1)
                      ? (m_width - 1.0f) / (image.getWidth() - 1.0f)
                      : 0.0f;
    const auto yScale = (image.getHeight() > 1)
                      ? (m_height - 1.0f) / (image.getHeight() - 1.0f)
                      : 0.0f;

    for (uint8_t frame = 0 ; frame < m_numberOfFrames ; ++frame)
    {
        for (int j = 0; j < image.getHeight(); ++j)
        {
            for (int i = 0; i < image.getWidth(); ++i)
            {
                const auto xMid = i * xScale;
                const auto yMid = j * yScale;

                const auto xLow = std::max(0, static_cast<int>(std::floor(xMid)) - a + 1);
                const auto xHigh = std::min(m_width - 1, static_cast<int>(std::floor(xMid)) + a);
                const auto yLow = std::max(0, static_cast<int>(std::floor(yMid)) - a + 1);
                const auto yHigh = std::min(m_height - 1, static_cast<int>(std::floor(yMid)) + a);

                float weightsSum{};
                float redSum{};
                float greenSum{};
                float blueSum{};

                for (int y = yLow; y <= yHigh; ++y)
                {
                    const auto dy = yMid - y;
                    const auto yKernelValue = kernel(dy, a);

                    for (int x = xLow; x <= xHigh; ++x)
                    {
                        const auto dx = xMid - x;
                        const auto weight = kernel(dx, a) * yKernelValue;
                        weightsSum += weight;

                        auto rgb = *getPixelRGB(Interface8880Point{x, y});
                        redSum += rgb.getRed() * weight;
                        greenSum += rgb.getGreen() * weight;
                        blueSum += rgb.getBlue() * weight;
                    }
                }

                const auto red = std::clamp(redSum / weightsSum, 0.0f, 255.0f);
                const auto green = std::clamp(greenSum / weightsSum, 0.0f, 255.0f);
                const auto blue = std::clamp(blueSum / weightsSum, 0.0f, 255.0f);

                RGB8880 rgb{static_cast<uint8_t>(red),
                            static_cast<uint8_t>(green),
                            static_cast<uint8_t>(blue)};

                image.setPixelRGB(Interface8880Point{i, j}, rgb, frame);
            }
        }
    }

    return image;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::Image8880::resizeToNearestNeighbour(
    fb32::Image8880& image) const
{
    const int a = (image.getWidth() > m_width) ? 0 : 1;
    const int b = (image.getHeight() > m_height) ? 0 : 1;


    for (uint8_t frame = 0 ; frame < m_numberOfFrames ; ++frame)
    {
        for (int j = 0 ; j < image.getHeight() ; ++j)
        {
            const int y = (j * (m_height - b)) / (image.getHeight() - b);
            for (int i = 0 ; i < image.getWidth() ; ++i)
            {
                const int x = (i * (m_width - a)) / (image.getWidth() - a);
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

fb32::Image8880
fb32::Image8880::scaleUp(
    uint8_t scale) const
{
    Image8880 image{m_width * scale, m_height * scale, m_numberOfFrames};

    for (uint8_t frame = 0 ; frame < m_numberOfFrames ; ++frame)
    {
        for (int j = 0 ; j < m_height ; ++j)
        {
            for (int i = 0 ; i < m_width ; ++i)
            {
                auto pixel = getPixel(Interface8880Point{i, j}, frame);
                for (int b = 0 ; b < scale ; ++b)
                {
                    for (int a = 0 ; a < scale ; ++a)
                    {
                        Interface8880Point p{ (i * scale) + a, (j * scale) + b};
                        image.setPixel(p, *pixel, frame);
                    }
                }
            }
        }
    }

    return image;
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

size_t
fb32::Image8880::offset(
    const Interface8880Point& p,
    uint8_t frame) const noexcept
{
    return p.x() + (p.y() * m_width) + (m_width * m_height * frame);
}

