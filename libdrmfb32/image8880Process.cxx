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

#ifdef WITH_BS_THREAD_POOL
#include "BS_thread_pool.hpp"
#endif

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
        const auto rgb8 = rgb.getRGB8();
        m_red += rgb8.red;
        m_green += rgb8.green;
        m_blue += rgb8.blue;
    }

    void subtract(const fb32::RGB8880& rgb) noexcept
    {
        const auto rgb8 = rgb.getRGB8();
        m_red -= rgb8.red;
        m_green -= rgb8.green;
        m_blue -= rgb8.blue;
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

#ifdef WITH_BS_THREAD_POOL

BS::thread_pool& threadPool()
{
    static BS::thread_pool s_threadPool;

    return s_threadPool;
}

#endif

//-------------------------------------------------------------------------

void
boxBlurRows(
    const fb32::Interface8880& input,
    fb32::Image8880& rb,
    int radius,
    int jStart,
    int jEnd)
{
    auto clamp = [](int value, int end) -> int
    {
        return std::clamp(value, 0, end - 1);
    };

    const auto diameter = 2 * radius + 1;
    const auto width = input.getWidth();
    auto inputi = input.getBuffer().data();
    auto rbi = rb.getBuffer().data();

    for (auto j = jStart ; j < jEnd ; ++j)
    {
        AccumulateRGB8880 argb;

        for (auto k = -radius - 1 ; k < radius ; ++k)
        {
            const Point p{clamp(k, width), j};
            argb.add(fb32::RGB8880(*(inputi + input.offset(p))));
        }

        for (auto i = 0 ; i < width ; ++i)
        {
            Point p{clamp(i + radius, width), j};
            argb.add(fb32::RGB8880(*(inputi + input.offset(p))));

            p = Point(clamp(i - radius - 1, width), j);
            argb.subtract(fb32::RGB8880(*(inputi + input.offset(p))));

            p = Point(i, j);
            *(rbi + rb.offset(p)) = argb.average(diameter).get8880();
        }
    }
}

//-------------------------------------------------------------------------

void
boxBlurColumns(
    const fb32::Image8880& rb,
    fb32::Image8880& output,
    int radius,
    int iStart,
    int iEnd)
{
    auto clamp = [](int value, int end) -> int
    {
        return std::clamp(value, 0, end - 1);
    };

    const auto diameter = 2 * radius + 1;
    const auto height = rb.getHeight();
    const auto rbi = rb.getBuffer().data();
    auto outputi = output.getBuffer().data();

    for (auto i = iStart ; i < iEnd ; ++i)
    {
        AccumulateRGB8880 argb;

        for (auto k = -radius - 1 ; k < radius ; ++k)
        {
            const Point p{i, clamp(k, height)};
            argb.add(fb32::RGB8880(*(rbi + rb.offset(p))));
        }

        for (auto j = 0 ; j < height ; ++j)
        {
            Point p{i, clamp(j + radius, height)};
            argb.add(fb32::RGB8880(*(rbi + rb.offset(p))));

            p = Point(i, clamp(j - radius - 1, height));
            argb.subtract(fb32::RGB8880(*(rbi + rb.offset(p))));

            p = Point(i, j);
            *(outputi + output.offset(p)) = argb.average(diameter).get8880();
        }
    }
}

//-------------------------------------------------------------------------

void
rowsBilinearInterpolation(
    const fb32::Interface8880& input,
    fb32::Image8880& output,
    int jStart,
    int jEnd)
{
    const auto xScale = (output.getWidth() > 1)
                      ? (input.getWidth() - 1.0f) / (output.getWidth() - 1.0f)
                      : 0.0f;
    const auto yScale = (output.getHeight() > 1)
                      ? (input.getHeight() - 1.0f) / (output.getHeight() - 1.0f)
                      : 0.0f;

    for (int j = jStart; j < jEnd; ++j)
    {
        for (int i = 0; i < output.getWidth(); ++i)
        {
            int xLow = static_cast<int>(std::floor(xScale * i));
            int yLow = static_cast<int>(std::floor(yScale * j));
            int xHigh = static_cast<int>(std::ceil(xScale * i));
            int yHigh = static_cast<int>(std::ceil(yScale * j));

            const auto xWeight = (xScale * i) - xLow;
            const auto yWeight = (yScale * j) - yLow;

            auto a = *input.getPixelRGB8(Point{xLow, yLow});
            auto b = *input.getPixelRGB8(Point{xHigh, yLow});
            auto c = *input.getPixelRGB8(Point{xLow, yHigh});
            auto d = *input.getPixelRGB8(Point{xHigh, yHigh});

            const auto aWeight = (1.0f - xWeight) * (1.0f - yWeight);
            const auto bWeight = xWeight * (1.0f - yWeight);
            const auto cWeight = (1.0f - xWeight) * yWeight;
            const auto dWeight = xWeight * yWeight;

            auto evaluate = [&](const uint8_t fb32::RGB8::* channel) -> uint8_t
            {
                float value = a.*channel * aWeight
                            + b.*channel * bWeight
                            + c.*channel * cWeight
                            + d.*channel * dWeight;

                return static_cast<uint8_t>(std::clamp(value, 0.0f, 255.0f));
            };

            fb32::RGB8880 rgb{evaluate(&fb32::RGB8::red),
                              evaluate(&fb32::RGB8::green),
                              evaluate(&fb32::RGB8::blue)};

            output.setPixelRGB(Point{i, j}, rgb);
        }
    }
}

//-------------------------------------------------------------------------

float
lanczosKernel(float x, int a)
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
}

//-------------------------------------------------------------------------

void
rowsLanczos3Interpolation(
    const fb32::Interface8880& input,
    fb32::Image8880& output,
    int jStart,
    int jEnd)
{
    constexpr int a{3};
    const auto xScale = (output.getWidth() > 1)
                      ? (input.getWidth() - 1.0f) / (output.getWidth() - 1.0f)
                      : 0.0f;
    const auto yScale = (output.getHeight() > 1)
                      ? (input.getHeight() - 1.0f) / (output.getHeight() - 1.0f)
                      : 0.0f;

    for (int j = jStart; j < jEnd; ++j)
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
                const auto yKernelValue = lanczosKernel(dy, a);

                for (int x = xLow; x <= xHigh; ++x)
                {
                    const auto dx = xMid - x;
                    const auto weight = lanczosKernel(dx, a) * yKernelValue;
                    weightsSum += weight;

                    auto rgb = *input.getPixelRGB(Point{x, y});
                    const auto rgb8 = rgb.getRGB8();
                    redSum += rgb8.red * weight;
                    greenSum += rgb8.green * weight;
                    blueSum += rgb8.blue * weight;
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
}

//-------------------------------------------------------------------------

void
rowsNearestNeighbour(
    const fb32::Interface8880& input,
    fb32::Image8880& output,
    int jStart,
    int jEnd)
{
    const auto inputWidth = input.getWidth();
    const auto inputHeight = input.getHeight();
    const auto outputWidth = output.getWidth();
    const auto outputHeight = output.getHeight();

    const int a = (outputWidth > inputWidth) ? 0 : 1;
    const int b = (output.getHeight() > inputHeight) ? 0 : 1;

    for (int j = jStart ; j < jEnd ; ++j)
    {
        const int y = (j * (inputHeight - b)) / (outputHeight - b);
        for (int i = 0 ; i < outputWidth ; ++i)
        {
            const int x = (i * (inputWidth - a)) / (outputWidth - a);
            auto pixel{input.getPixel(Point{x, y})};

            if (pixel.has_value())
            {
                output.setPixel(Point{i, j}, pixel.value());
            }
        }
    }
}

//-------------------------------------------------------------------------

void
rowsScaleUp(
    const fb32::Interface8880& input,
    fb32::Image8880& output,
    uint8_t scale,
    int jStart,
    int jEnd)
{
    const auto width = input.getWidth();
    auto inputi = input.getBuffer().data() + (jStart * width);

    for (auto j = jStart ; j < jEnd ; ++j)
    {
        for (int i = 0 ; i < width ; ++i)
        {
            auto pixel = *(inputi++);
            for (int b = 0 ; b < scale ; ++b)
            {
                for (int a = 0 ; a < scale ; ++a)
                {
                    Point p{(i * scale) + a, (j * scale) + b};
                    output.setPixel(p, pixel);
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

void
rowsRotate(
    fb32::Image8880& image,
    fb32::Image8880& output,
    double sinAngle,
    double cosAngle,
    int jStart,
    int jEnd)
{
    const auto inputHeight = image.getHeight();
    const auto outputWidth = output.getWidth();

    const auto y00 = inputHeight * cosAngle;

    for (int j = jStart ; j < jEnd ; ++j)
    {
        const auto b = y00 - j;
        const auto bSinAngle = b * sinAngle;
        const auto bCosAngle = b * cosAngle;

        for (int i = 0 ; i < outputWidth ; ++i)
        {
            const auto x = (i * cosAngle) - bSinAngle;
            const auto y = (i * sinAngle) + bCosAngle;

            const auto x0 = static_cast<int>(floor(x));
            const auto y0 = static_cast<int>(floor(y));

            const auto x1 = static_cast<int>(ceil(x));
            const auto y1 = static_cast<int>(ceil(y));

            const auto pixel00 = image.getPixel(Point{x0, inputHeight - 1 - y0});
            const auto pixel01 = image.getPixel(Point{x0, inputHeight - 1 - y1});
            const auto pixel10 = image.getPixel(Point{x1, inputHeight - 1 - y0});
            const auto pixel11 = image.getPixel(Point{x1, inputHeight - 1 - y1});

            if (pixel00.has_value() and
                pixel01.has_value() and
                pixel10.has_value() and
                pixel11.has_value())
            {
                const auto xWeight = x - x0;
                const auto yWeight = y - y0;

                const auto aWeight = (1.0 - xWeight) * (1.0 - yWeight);
                const auto bWeight = (1.0 - xWeight) * yWeight;
                const auto cWeight = xWeight * (1.0 - yWeight);
                const auto dWeight = xWeight * yWeight;

                auto evaluate = [&](const uint8_t fb32::RGB8::* channel) -> uint8_t
                {
                    double value = fb32::RGB8(pixel00.value()).*channel * aWeight
                                 + fb32::RGB8(pixel01.value()).*channel * bWeight
                                 + fb32::RGB8(pixel10.value()).*channel * cWeight
                                 + fb32::RGB8(pixel11.value()).*channel * dWeight;

                    return static_cast<uint8_t>(std::clamp(value, 0.0, 255.0));
                };

                fb32::RGB8880 rgb{evaluate(&fb32::RGB8::red),
                                  evaluate(&fb32::RGB8::green),
                                  evaluate(&fb32::RGB8::blue)};

                output.setPixel(Point{i, j}, rgb.get8880());
            }
        }
    }
}


//-------------------------------------------------------------------------

}

//=========================================================================


fb32::Image8880
fb32::boxBlur(
    const fb32::Interface8880& input,
    int radius)
{
    const auto width = input.getWidth();
    const auto height = input.getHeight();

    Image8880 rb{width, height};
    Image8880 output{width, height};

#ifdef WITH_BS_THREAD_POOL

    auto& tPool = threadPool();

    auto iterateRows = [&input, &rb, radius](int start, int end)
    {
        boxBlurRows(input, rb, radius, start, end);
    };

    tPool.detach_blocks<int>(0, height, iterateRows);
    tPool.wait();

    auto iterateColumns = [&rb, &output, radius](int start, int end)
    {
        boxBlurColumns(rb, output, radius, start, end);
    };

    tPool.detach_blocks<int>(0, width, iterateColumns);
    tPool.wait();

#else

    boxBlurRows(input, rb, radius, 0, height);
    boxBlurColumns(rb, output, radius, 0, width);

#endif

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

    const auto mb = boxBlur(maxRGB(input), 12);

    Image8880 output{input.getWidth(), input.getHeight()};

    const auto strength2 = strength * strength;
    const auto minI = 1.0 / flerp(1.0, 10.0, strength2);
    const auto maxI = 1.0 / flerp(1.0, 1.111, strength2);

    auto mbi = mb.getBuffer().data();
    auto outputi = output.getBuffer().data();

    for (auto pixel : input.getBuffer())
    {
        RGB8880 c{pixel};
        const auto rgb8 = c.getRGB8();
        const auto max = RGB8(*(mbi++)).red;
        const auto illumination = std::clamp(max / 255.0, minI, maxI);

        if (illumination < maxI)
        {
            const auto r = illumination / maxI;
            const auto scale = (0.4 + (r * 0.6)) / r;

            c.setRGB(scaled(rgb8.red, scale),
                     scaled(rgb8.green, scale),
                     scaled(rgb8.blue, scale));
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
    Image8880 output{input.getWidth(), input.getHeight()};
    auto* buffer = output.getBuffer().data();

    for (const auto pixel : input.getBuffer())
    {
        RGB8 rgb8(pixel);
        const auto grey(std::max({rgb8.red, rgb8.green, rgb8.blue}));
        *(buffer++) = RGB8880::rgbTo8880(grey, grey, grey);
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

    Image8880 output{width, height};
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

    Image8880 output{width, height};
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

    Image8880 output{width, height};
    resizeToNearestNeighbour(input, output);

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::resizeToBilinearInterpolation(
    const fb32::Interface8880& input,
    fb32::Image8880& output)
{
#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output](int start, int end)
    {
        rowsBilinearInterpolation(input, output, start, end);
    };

    tPool.detach_blocks<int>(0, output.getHeight(), iterateRows);
    tPool.wait();
#else
    rowsBilinearInterpolation(input, output, 0, output.getHeight());
#endif
    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::resizeToLanczos3Interpolation(
    const fb32::Interface8880& input,
    fb32::Image8880& output)
{
#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output](int start, int end)
    {
        rowsLanczos3Interpolation(input, output, start, end);
    };

    tPool.detach_blocks<int>(0, output.getHeight(), iterateRows);
    tPool.wait();
#else
    rowsLanczos3Interpolation(input, output, 0, output.getHeight());
#endif
    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::resizeToNearestNeighbour(
    const fb32::Interface8880& input,
    fb32::Image8880& output)
{
#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output](int start, int end)
    {
        rowsNearestNeighbour(input, output, start, end);
    };

    tPool.detach_blocks<int>(0, output.getHeight(), iterateRows);
    tPool.wait();
#else
    rowsNearestNeighbour(input, output, 0, output.getHeight());
#endif

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::rotate(
    const fb32::Interface8880& input,
    uint32_t background,
    double angle)
{
    if (angle >= 360.0)
    {
        angle = fmod(angle, 360.0);
    }
    else if (angle < 0.0)
    {
        angle = 360.0 + fmod(angle, 360.0);
    }

    // rotate so angle is in the range 0 to 90

    Image8880 image;

    if (angle >= 270.0)
    {
        image = rotate270(input);
        angle -= 270.0;
    }
    else if (angle >= 180.0)
    {
        image = rotate180(input);
        angle -= 180.0;
    }
    else if (angle >= 90.0)
    {
        image = rotate90(input);
        angle -= 90.0;
    }
    else
    {
        image = Image8880(input.getWidth(), input.getHeight(), input.getBuffer());
    }

    // now angle is in the range 0 to 90
    if (std::min(angle, 90.0 - angle) < 0.01)
    {
        return image;
    }

    //---------------------------------------------------------------------
    //
    // (x0,y0) +-------+ (x1,y0)
    //         |       |
    //         |       |
    //         |       |
    // (x0,y1) +-------+ (x1,y1)
    //
    // x' =  x * cos(angle) + y * sin(angle)
    // y' = -x * sin(angle) + y * cos(angle)
    //
    // x = x' * cos(angle) - y' * sin(angle)
    // y = x' * sin(angle) + y' * cos(angle)
    //
    //---------------------------------------------------------------------

    const auto radians = angle * (std::numbers::pi_v<double> / 180.0);
    const auto cosAngle = std::cos(radians);
    const auto sinAngle = std::sin(radians);

    const auto inputWidth = image.getWidth();
    const auto inputHeight = image.getHeight();

    const auto x10 = (inputWidth * cosAngle) + (inputHeight * sinAngle);
    const auto y00 = inputHeight * cosAngle;
    const auto y11 = -(inputWidth * sinAngle);

    const auto outputWidth = static_cast<int>(std::ceil(x10));
    const auto outputHeight = static_cast<int>(std::ceil(y00 - y11 + 1.0));

    Image8880 output{outputWidth, outputHeight};
    output.clear(background);

#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&image, &output, sinAngle, cosAngle](int start, int end)
    {
        rowsRotate(image, output, sinAngle, cosAngle, start, end);
    };

    tPool.detach_blocks<int>(0, output.getHeight(), iterateRows);
    tPool.wait();
#else
    rowsRotate(image, output, sinAngle, cosAngle, 0, output.getHeight());
#endif

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::rotate90(
    const fb32::Interface8880& input)
{
    const auto width = input.getWidth();
    const auto height = input.getHeight();
    Image8880 output{height, width};

    for (auto j = 0 ; j < height ; ++j)
    {
        for (auto i = 0 ; i < width ; ++i)
        {
            const auto pixel{input.getPixel(Point{i, j})};

            if (pixel.has_value())
            {
                output.setPixel(Point{height - j - 1, i}, pixel.value());
            }
        }
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::rotate180(
    const fb32::Interface8880& input)
{
    const auto width = input.getWidth();
    const auto height = input.getHeight();
    Image8880 output{width, height};

    for (auto j = 0 ; j < height ; ++j)
    {
        for (auto i = 0 ; i < width ; ++i)
        {
            const auto pixel{input.getPixel(Point{i, j})};

            if (pixel.has_value())
            {
                output.setPixel(Point{width - i - 1, height - j - 1}, pixel.value());
            }
        }
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::rotate270(
    const fb32::Interface8880& input)
{
    const auto width = input.getWidth();
    const auto height = input.getHeight();
    Image8880 output{height, width};

    for (auto j = 0 ; j < height ; ++j)
    {
        for (auto i = 0 ; i < width ; ++i)
        {
            const auto pixel{input.getPixel(Point{i, j})};

            if (pixel.has_value())
            {
                output.setPixel(Point{j, width - i - 1}, pixel.value());
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
    const auto width = input.getWidth();
    const auto height = input.getHeight();
    Image8880 output{width * scale, height * scale};

#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output, scale](int start, int end)
    {
        rowsScaleUp(input, output, scale, start, end);
    };

    tPool.detach_blocks<int>(0, height, iterateRows);
    tPool.wait();
#else
    rowsScaleUp(input, output, scale, 0, height);
#endif

    return output;
}

