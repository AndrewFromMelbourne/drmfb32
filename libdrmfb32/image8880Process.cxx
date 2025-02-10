//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2025 Andrew Duncan
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
#include "image8880Process.h"

//-------------------------------------------------------------------------

using size_type = std::vector<uint32_t>::size_type;
using Point = fb32::Interface8880Point;

//=========================================================================

namespace {

//-------------------------------------------------------------------------

class AccumulateRGB8880
{
public:

    void add(const fb32::RGB8880& rgb) noexcept
    {
        m_red += rgb.getRed();
        m_green += rgb.getGreen();
        m_blue += rgb.getBlue();
    }

    void subtract(const fb32::RGB8880& rgb) noexcept
    {
        m_red -= rgb.getRed();
        m_green -= rgb.getGreen();
        m_blue -= rgb.getBlue();
    }

    [[nodiscard]] fb32::RGB8880 average(int count) noexcept
    {
        return fb32::RGB8880{static_cast<uint8_t>(m_red / count),
                             static_cast<uint8_t>(m_green / count),
                             static_cast<uint8_t>(m_blue / count)};
    }

private:

    int m_red{0};
    int m_green{0};
    int m_blue{0};
};

//-------------------------------------------------------------------------

}

//=========================================================================

fb32::Image8880
fb32::boxBlur(
    const fb32::Interface8880& input,
    int radius)
{
    auto clamp = [](int value, int end) -> int
    {
        return std::clamp(value, 0, end - 1);
    };

    const auto diameter = 2 * radius + 1;

    // row blurred image
    fb32::Image8880 rb{input.getWidth(), input.getHeight()};

    auto inputi = input.getBuffer().begin();
    auto rbi = rb.getBuffer().begin();

    for (auto j = 0 ; j < input.getHeight() ; ++j)
    {
        AccumulateRGB8880 argb;

        for (auto k = -radius - 1 ; k < radius ; ++k)
        {
            const Point p{clamp(k, input.getWidth()), j};
            argb.add(fb32::RGB8880(*(inputi + input.offset(p))));
        }

        for (auto i = 0 ; i < input.getWidth() ; ++i)
        {
            Point p{clamp(i + radius, input.getWidth()), j};
            argb.add(fb32::RGB8880(*(inputi + input.offset(p))));

            p = Point(clamp(i - radius - 1, input.getWidth()), j);
            argb.subtract(fb32::RGB8880(*(inputi + input.offset(p))));

            p = Point(i, j);
            *(rbi + rb.offset(p)) = argb.average(diameter).get8880();
        }
    }

    fb32::Image8880 output{rb.getWidth(), rb.getHeight()};
    auto outputi = output.getBuffer().begin();

    for (auto i = 0 ; i < input.getWidth() ; ++i)
    {
        AccumulateRGB8880 argb;

        for (auto k = -radius - 1 ; k < radius ; ++k)
        {
            const Point p{i, clamp(k, rb.getHeight())};
            argb.add(fb32::RGB8880(*(rbi + rb.offset(p))));
        }

        for (auto j = 0 ; j < rb.getHeight() ; ++j)
        {
            Point p{i, clamp(j + radius, rb.getHeight())};
            argb.add(fb32::RGB8880(*(rbi + rb.offset(p))));

            p = Point(i, clamp(j - radius - 1, rb.getHeight()));
            argb.subtract(fb32::RGB8880(*(rbi + rb.offset(p))));

            p = Point(i, j);
            *(outputi + output.offset(p)) = argb.average(diameter).get8880();
        }
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::enlighten(
    const fb32::Interface8880& input,
    double strength)
{
    auto flerp = [](double value1, double value2, double alpha)->double
    {
        return (value1 * (1.0 - alpha)) + (value2 * alpha);
    };

    auto scaled = [](uint8_t channel, double scale)->uint8_t
    {
        return static_cast<uint8_t>(std::clamp(channel * scale, 0.0, 255.0));
    };

    const auto mb = fb32::boxBlur(fb32::maxRGB(input), 12);

    fb32::Image8880 output{input.getWidth(), input.getHeight()};

    const auto strength2 = strength * strength;
    const auto minI = 1.0 / flerp(1.0, 10.0, strength2);
    const auto maxI = 1.0 / flerp(1.0, 1.111, strength2);

    auto mbi = mb.getBuffer().begin();
    auto outputi = output.getBuffer().begin();

    for (auto pixel : input.getBuffer())
    {
        auto c = fb32::RGB8880(pixel);
        auto max = fb32::RGB8880(*(mbi++)).getRed();
        const auto illumination = std::clamp(max / 255.0, minI, maxI);

        if (illumination < maxI)
        {
            const auto r = illumination / maxI;
            const auto scale = (0.4 + (r * 0.6)) / r;

            c.setRGB(scaled(c.getRed(), scale),
                     scaled(c.getGreen(), scale),
                     scaled(c.getBlue(), scale));
        }

        *(outputi++) = c.get8880();
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::maxRGB(
    const fb32::Interface8880& input)
{
    fb32::Image8880 output{input.getWidth(), input.getHeight()};
    auto* buffer = output.getBuffer().data();

    for (const auto pixel : input.getBuffer())
    {
        fb32::RGB8880 rgb(pixel);
        rgb.setGrey(std::max({rgb.getRed(), rgb.getGreen(), rgb.getBlue()}));
        *(buffer++) = rgb.get8880();
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::resizeBilinearInterpolation(
    const fb32::Interface8880& input,
    int width,
    int height)
{
    if ((width <= 0) or (height <= 0))
    {
        throw std::invalid_argument("width and height must be greater than zero");
    }

    fb32::Image8880 output{width, height};
    resizeToBilinearInterpolation(input, output);

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::resizeLanczos3Interpolation(
    const fb32::Interface8880& input,
    int width,
    int height)
{
    if ((width <= 0) or (height <= 0))
    {
        throw std::invalid_argument("width and height must be greater than zero");
    }

    fb32::Image8880 output{width, height};
    resizeToLanczos3Interpolation(input, output);

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::resizeNearestNeighbour(
    const fb32::Interface8880& input,
    int width,
    int height)
{
    if ((width <= 0) or (height <= 0))
    {
        throw std::invalid_argument("width and height must be greater than zero");
    }

    fb32::Image8880 output{width, height};
    resizeToNearestNeighbour(input, output);

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::resizeToBilinearInterpolation(
    const fb32::Interface8880& input,
    fb32::Image8880& output)
{
    const auto xScale = (output.getWidth() > 1)
                      ? (input.getWidth() - 1.0f) / (output.getWidth() - 1.0f)
                      : 0.0f;
    const auto yScale = (output.getHeight() > 1)
                      ? (input.getHeight() - 1.0f) / (output.getHeight() - 1.0f)
                      : 0.0f;

    for (int j = 0; j < output.getHeight(); ++j)
    {
        for (int i = 0; i < output.getWidth(); ++i)
        {
            int xLow = static_cast<int>(std::floor(xScale * i));
            int yLow = static_cast<int>(std::floor(yScale * j));
            int xHigh = static_cast<int>(std::ceil(xScale * i));
            int yHigh = static_cast<int>(std::ceil(yScale * j));

            const auto xWeight = (xScale * i) - xLow;
            const auto yWeight = (yScale * j) - yLow;

            auto a = *input.getPixelRGB(Point{xLow, yLow});
            auto b = *input.getPixelRGB(Point{xHigh, yLow});
            auto c = *input.getPixelRGB(Point{xLow, yHigh});
            auto d = *input.getPixelRGB(Point{xHigh, yHigh});

            const auto aWeight = (1.0f - xWeight) * (1.0f - yWeight);
            const auto bWeight = xWeight * (1.0f - yWeight);
            const auto cWeight = (1.0f - xWeight) * yWeight;
            const auto dWeight = xWeight * yWeight;

            typedef  uint8_t (fb32::RGB8880::*RGB8880MemFn)() const;

            auto evaluate = [&](RGB8880MemFn get) -> uint8_t
            {
                float value = std::invoke(get, a) * aWeight +
                                std::invoke(get, b) * bWeight +
                                std::invoke(get, c) * cWeight +
                                std::invoke(get, d) * dWeight;

                return static_cast<uint8_t>(std::clamp(value, 0.0f, 255.0f));
            };

            fb32::RGB8880 rgb{evaluate(&fb32::RGB8880::getRed),
                              evaluate(&fb32::RGB8880::getGreen),
                              evaluate(&fb32::RGB8880::getBlue)};

            output.setPixelRGB(Point{i, j}, rgb);
        }
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::resizeToLanczos3Interpolation(
    const fb32::Interface8880& input,
    fb32::Image8880& output)
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

    const auto xScale = (output.getWidth() > 1)
                      ? (input.getWidth() - 1.0f) / (output.getWidth() - 1.0f)
                      : 0.0f;
    const auto yScale = (output.getHeight() > 1)
                      ? (input.getHeight() - 1.0f) / (output.getHeight() - 1.0f)
                      : 0.0f;

    for (int j = 0; j < output.getHeight(); ++j)
    {
        for (int i = 0; i < output.getWidth(); ++i)
        {
            const auto xMid = i * xScale;
            const auto yMid = j * yScale;

            const auto xLow = std::max(0, static_cast<int>(std::floor(xMid)) - a + 1);
            const auto xHigh = std::min(input.getWidth() - 1, static_cast<int>(std::floor(xMid)) + a);
            const auto yLow = std::max(0, static_cast<int>(std::floor(yMid)) - a + 1);
            const auto yHigh = std::min(input.getHeight() - 1, static_cast<int>(std::floor(yMid)) + a);

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

                    auto rgb = *input.getPixelRGB(Point{x, y});
                    redSum += rgb.getRed() * weight;
                    greenSum += rgb.getGreen() * weight;
                    blueSum += rgb.getBlue() * weight;
                }
            }

            const auto red = std::clamp(redSum / weightsSum, 0.0f, 255.0f);
            const auto green = std::clamp(greenSum / weightsSum, 0.0f, 255.0f);
            const auto blue = std::clamp(blueSum / weightsSum, 0.0f, 255.0f);

            fb32::RGB8880 rgb{static_cast<uint8_t>(red),
                              static_cast<uint8_t>(green),
                              static_cast<uint8_t>(blue)};

            output.setPixelRGB(Point{i, j}, rgb);
        }
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::resizeToNearestNeighbour(
    const fb32::Interface8880& input,
    fb32::Image8880& output)
{
    const int a = (output.getWidth() > input.getWidth()) ? 0 : 1;
    const int b = (output.getHeight() > input.getHeight()) ? 0 : 1;

    for (int j = 0 ; j < output.getHeight() ; ++j)
    {
        const int y = (j * (input.getHeight() - b)) / (output.getHeight() - b);
        for (int i = 0 ; i < output.getWidth() ; ++i)
        {
            const int x = (i * (input.getWidth() - a)) / (output.getWidth() - a);
            auto pixel{input.getPixel(Point{x, y})};

            if (pixel.has_value())
            {
                output.setPixel(Point{i, j}, pixel.value());
            }
        }
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::scaleUp(
    const fb32::Interface8880& input,
    uint8_t scale)
{
    fb32::Image8880 output{input.getWidth() * scale, input.getHeight() * scale};

    for (int j = 0 ; j < input.getHeight() ; ++j)
    {
        for (int i = 0 ; i < input.getWidth() ; ++i)
        {
            auto pixel = input.getPixel(Point{i, j});
            for (int b = 0 ; b < scale ; ++b)
            {
                for (int a = 0 ; a < scale ; ++a)
                {
                    Point p{ (i * scale) + a, (j * scale) + b};
                    output.setPixel(p, *pixel);
                }
            }
        }
    }

    return output;
}

