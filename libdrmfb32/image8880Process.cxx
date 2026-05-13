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
#include <mutex>
#include <numbers>
#include <stdexcept>

#include "image8880.h"
#include "image8880Process.h"

#ifdef WITH_BS_THREAD_POOL
#include "BS_thread_pool.hpp"
#endif

//-------------------------------------------------------------------------

using size_type = std::vector<uint32_t>::size_type;
using Point = fb32::Point8880;

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

class CountIntensity
{
public:

    void add(const fb32::RGB8880& rgb) noexcept
    {
        const auto intensity = rgb.toIntensity();
        m_intensity[intensity]++;
    }

    void add(const CountIntensity& other) noexcept
    {
        for (int i = 0 ; i < 256 ; ++i)
        {
            m_intensity[i] += other.m_intensity[i];
        }
    }

    void count(const fb32::Interface8880Base& input);

    [[nodiscard]] int count(int i) const noexcept
    {
        if (i < 0 or i > 255)
        {
            return 0;
        }

        return m_intensity[i];
    }


    [[nodiscard]] fb32::Image8880 histogram() const noexcept
    {
        constexpr fb32::RGB8880 grey{15, 15, 15};
        constexpr fb32::RGB8880 white{255, 255, 255};

        fb32::Image8880 image({256, 256});
        image.clear(grey);

        const auto max = maximum();

        if (max == 0)
        {
            return {};
        }

        for (int i = 0 ; i < 256 ; ++i)
        {
            const auto value = static_cast<uint8_t>((m_intensity[i] * 255) / max);

            for (int j = 0 ; j < 256 ; ++j)
            {
                if (value >= j)
                {
                    image.setPixelRGB(Point{i, 255 - j}, white);
                }
            }
        }

        return image;
    }

    [[nodiscard]] int maximum() const noexcept
    {
        return *std::max_element(m_intensity.begin(), m_intensity.end());
    }

private:

    std::array<int, 256> m_intensity{};
};

//-------------------------------------------------------------------------

class CountRGB
{
public:

    void add(const fb32::RGB8880& rgb) noexcept
    {
        const auto rgb8 = rgb.getRGB8();
        m_red[rgb8.red]++;
        m_green[rgb8.green]++;
        m_blue[rgb8.blue]++;
    }

    void add(const CountRGB& other) noexcept
    {
        for (int i = 0 ; i < 256 ; ++i)
        {
            m_red[i] += other.m_red[i];
            m_green[i] += other.m_green[i];
            m_blue[i] += other.m_blue[i];
        }
    }

    void count(const fb32::Interface8880Base& input);

    [[nodiscard]] fb32::Image8880 histogram() const noexcept
    {
        constexpr fb32::RGB8880 grey{15, 15, 15};

        fb32::Image8880 image({256, 256});
        image.clear(grey);

        const auto max = maximum();

        if (max == 0)
        {
            return {};
        }

        for (int i = 0 ; i < 256 ; ++i)
        {
            const auto maxRed = static_cast<uint8_t>((m_red[i] * 255) / max);
            const auto maxGreen = static_cast<uint8_t>((m_green[i] * 255) / max);
            const auto maxBlue = static_cast<uint8_t>((m_blue[i] * 255) / max);

            for (int j = 0 ; j < 256 ; ++j)
            {
                const auto red = (maxRed >= j) ? 255 : 0;
                const auto green = (maxGreen >= j) ? 255 : 0;
                const auto blue = (maxBlue >= j) ? 255 : 0;

                if (red == 0 and green == 0 and blue == 0)
                {
                    break;
                }

                image.setPixelRGB(Point{i, 255 - j}, fb32::RGB8880(red, green, blue));
            }
        }

        return image;
    }

    [[nodiscard]] int maximum() const noexcept
    {
        auto maxRed = std::max_element(m_red.begin(), m_red.end());
        auto maxGreen = std::max_element(m_green.begin(), m_green.end());
        auto maxBlue = std::max_element(m_blue.begin(), m_blue.end());

        return std::max({*maxRed, *maxGreen, *maxBlue});
    }

private:

    std::array<int, 256> m_red{};
    std::array<int, 256> m_green{};
    std::array<int, 256> m_blue{};
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
    const fb32::Interface8880Base& input,
    fb32::Image8880& rb,
    int radius,
    int jStart,
    int jEnd)
{
    auto clamp = [](int value, int end) -> int
    {
        return std::clamp(value, 0, end - 1);
    };

    const auto d = input.getDimensions();

    const auto diameter = 2 * radius + 1;
    const auto width = d.width();
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

    const auto d = rb.getDimensions();
    const auto diameter = 2 * radius + 1;
    const auto height = d.height();
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
rowsCountIntensity(
    const fb32::Interface8880Base& input,
    CountIntensity& count,
    int jStart,
    int jEnd)
{
    for (int j = jStart ; j < jEnd ; ++j)
    {
        const auto row = input.getRow(j);

        for (const auto value : row)
        {
            auto pixel = fb32::RGB8880(value);
            count.add(pixel);
        }
    }
}

//-------------------------------------------------------------------------

void
rowsCountRGB(
    const fb32::Interface8880Base& input,
    CountRGB& count,
    int jStart,
    int jEnd)
{
    for (int j = jStart ; j < jEnd ; ++j)
    {
        const auto row = input.getRow(j);
        for (const auto value : row)
        {
            auto pixel = fb32::RGB8880(value);
            count.add(pixel);
        }
    }
}

//-------------------------------------------------------------------------

void
rowsBilinearInterpolation(
    const fb32::Interface8880Base& input,
    fb32::Image8880& output,
    int jStart,
    int jEnd)
{
    const auto id = input.getDimensions();
    const auto od = output.getDimensions();
    const auto xScale = (od.width() > 1)
                      ? (id.width() - 1.0f) / (od.width() - 1.0f)
                      : 0.0f;
    const auto yScale = (od.height() > 1)
                      ? (id.height() - 1.0f) / (od.height() - 1.0f)
                      : 0.0f;

    for (int j = jStart; j < jEnd; ++j)
    {
        for (int i = 0; i < od.width(); ++i)
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

void
rowsHistogramStretch(
    int low,
    int high,
    const fb32::Interface8880Base& input,
    fb32::Image8880& output,
    int jStart,
    int jEnd)
{
    std::array<uint8_t, 256> stretch{};

    for (int i = 0; i < 256; ++i)
    {
        if (i < low)
        {
            stretch[i] = 0;
        }
        else if (i > high)
        {
            stretch[i] = 255;
        }
        else
        {
            stretch[i] = static_cast<uint8_t>(((i - low) * 255) / (high - low));
        }
    }

    for (int j = jStart; j < jEnd; ++j)
    {
        const auto row = input.getRow(j);
        auto outputIterator = output.getRow(j).begin();

        for (auto pixel : row)
        {
            const auto rgb = fb32::RGB8(pixel);

            *(outputIterator++) = fb32::RGB8880{stretch[rgb.red],
                                                stretch[rgb.green],
                                                stretch[rgb.blue]}.get8880();
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
    const fb32::Interface8880Base& input,
    fb32::Image8880& output,
    int jStart,
    int jEnd)
{
    const auto id = input.getDimensions();
    const auto od = output.getDimensions();
    constexpr int a{3};
    const auto xScale = (od.width() > 1)
                      ? (id.width() - 1.0f) / (od.width() - 1.0f)
                      : 0.0f;
    const auto yScale = (od.height() > 1)
                      ? (id.height() - 1.0f) / (od.height() - 1.0f)
                      : 0.0f;

    for (int j = jStart; j < jEnd; ++j)
    {
        for (int i = 0; i < od.width(); ++i)
        {
            const auto xMid = i * xScale;
            const auto yMid = j * yScale;

            const auto xLow = std::max(0, static_cast<int>(std::floor(xMid)) - a + 1);
            const auto xHigh = std::min(id.width() - 1, static_cast<int>(std::floor(xMid)) + a);
            const auto yLow = std::max(0, static_cast<int>(std::floor(yMid)) - a + 1);
            const auto yHigh = std::min(id.height() - 1, static_cast<int>(std::floor(yMid)) + a);

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
    const fb32::Interface8880Base& input,
    fb32::Image8880& output,
    int jStart,
    int jEnd)
{
    const auto id = input.getDimensions();
    const auto od = output.getDimensions();
    const int a = (od.width() > id.width()) ? 0 : 1;
    const int b = (od.height() > id.height()) ? 0 : 1;

    for (int j = jStart ; j < jEnd ; ++j)
    {
        const int y = (j * (id.height() - b)) / (od.height() - b);
        for (int i = 0 ; i < od.width() ; ++i)
        {
            const int x = (i * (id.width() - a)) / (od.width() - a);
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
    const fb32::Interface8880Base& input,
    fb32::Image8880& output,
    uint8_t scale,
    int jStart,
    int jEnd)
{
    const auto id = input.getDimensions();
    auto inputi = input.getBuffer().data() + (jStart * id.width());

    for (auto j = jStart ; j < jEnd ; ++j)
    {
        for (int i = 0 ; i < id.width() ; ++i)
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
    const fb32::Image8880& image,
    fb32::Image8880& output,
    double sinAngle,
    double cosAngle,
    int jStart,
    int jEnd)
{
    const auto id = image.getDimensions();
    const auto od = output.getDimensions();

    const auto y00 = id.height() * cosAngle;

    for (int j = jStart ; j < jEnd ; ++j)
    {
        const auto b = y00 - j;
        const auto bSinAngle = b * sinAngle;
        const auto bCosAngle = b * cosAngle;

        for (int i = 0 ; i < od.width() ; ++i)
        {
            const auto x = (i * cosAngle) - bSinAngle;
            const auto y = (i * sinAngle) + bCosAngle;

            const auto x0 = static_cast<int>(floor(x));
            const auto y0 = static_cast<int>(floor(y));

            const auto x1 = static_cast<int>(ceil(x));
            const auto y1 = static_cast<int>(ceil(y));

            const auto pixel00 = image.getPixel(Point{x0, id.height() - 1 - y0});
            const auto pixel01 = image.getPixel(Point{x0, id.height() - 1 - y1});
            const auto pixel10 = image.getPixel(Point{x1, id.height() - 1 - y0});
            const auto pixel11 = image.getPixel(Point{x1, id.height() - 1 - y1});

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

void
rowsToGrey(
    const fb32::Interface8880Base& input,
    fb32::Image8880& output,
    int jStart,
    int jEnd)
{
    const auto id = input.getDimensions();
    auto inputi = input.getBuffer().data() + (jStart * id.width());

    for (auto j = jStart ; j < jEnd ; ++j)
    {
        for (int i = 0 ; i < id.width() ; ++i)
        {
            auto pixel = *(inputi++);
            Point p{i, j};
            output.setPixel(p,fb32::RGB8880(pixel).toGrey().get8880());
        }
    }
}

//-------------------------------------------------------------------------

void
CountIntensity::count(
    const fb32::Interface8880Base& input)
{
    const auto d = input.getDimensions();

#ifdef WITH_BS_THREAD_POOL
    std::mutex countMutex;
    auto& tPool = threadPool();
    auto iterateRows = [this, &input, &countMutex](int start, int end)
    {
        CountIntensity localCount;
        rowsCountIntensity(input, localCount, start, end);
        std::lock_guard<std::mutex> lock(countMutex);
        add(localCount);
    };

    tPool.detach_blocks<int>(0, d.height(), iterateRows);
    tPool.wait();
#else
    CountIntensity localCount;
    rowsCountIntensity(input, localCount, 0, d.height());
    add(localCount);
#endif
}

//-------------------------------------------------------------------------

void
CountRGB::count(
    const fb32::Interface8880Base& input)
{
    const auto d = input.getDimensions();

#ifdef WITH_BS_THREAD_POOL
    std::mutex countMutex;
    auto& tPool = threadPool();
    auto iterateRows = [this, &input, &countMutex](int start, int end)
    {
        CountRGB localCount;
        rowsCountRGB(input, localCount, start, end);
        std::lock_guard<std::mutex> lock(countMutex);
        add(localCount);
    };

    tPool.detach_blocks<int>(0, d.height(), iterateRows);
    tPool.wait();
#else
    CountRGB localCount;
    rowsCountRGB(input, localCount, 0, d.height());
    add(localCount);
#endif
}

//-------------------------------------------------------------------------

};

//=========================================================================

fb32::Image8880
fb32::boxBlur(
    const fb32::Interface8880Base& input,
    int radius)
{
    const auto d = input.getDimensions();
    Image8880 rb{d};
    Image8880 output{d};

#ifdef WITH_BS_THREAD_POOL

    auto& tPool = threadPool();

    auto iterateRows = [&input, &rb, radius](int start, int end)
    {
        boxBlurRows(input, rb, radius, start, end);
    };

    tPool.detach_blocks<int>(0, d.height(), iterateRows);
    tPool.wait();

    auto iterateColumns = [&rb, &output, radius](int start, int end)
    {
        boxBlurColumns(rb, output, radius, start, end);
    };

    tPool.detach_blocks<int>(0, d.width(), iterateColumns);
    tPool.wait();

#else

    boxBlurRows(input, rb, radius, 0, d.height());
    boxBlurColumns(rb, output, radius, 0, d.width());

#endif

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::enlighten(
    const fb32::Interface8880Base& input,
    double strength)
{
    const auto d = input.getDimensions();
    auto flerp = [](double value1, double value2, double alpha)->double
    {
        return (value1 * (1.0 - alpha)) + (value2 * alpha);
    };

    auto scaled = [](uint8_t channel, double scale)->uint8_t
    {
        return static_cast<uint8_t>(std::clamp(channel * scale, 0.0, 255.0));
    };

    const auto mb = boxBlur(maxRGB(input), 12);

    Image8880 output{d};

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
fb32::histogramIntensity(
    const Interface8880Base& input)
{
    CountIntensity count;
    count.count(input);

    return count.histogram();
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::histogramRGB(
    const Interface8880Base& input)
{
    CountRGB count;
    count.count(input);

    return count.histogram();
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::histogramStretch(
    int percent,
    const Interface8880Base& input)
{
    CountIntensity count;
    count.count(input);

    const auto d = input.getDimensions();
    const auto totalPixels = d.width() * d.height();
    const auto threshold = (totalPixels * percent) / (100 * 256);

    int low{};
    int high{255};

    for (int i = 0 ; i < 256 ; ++i)
    {
        if (count.count(i) > threshold)
        {
            low = i;
            break;
        }
    }

    for (int i = 255 ; i >= 0 ; --i)
    {
        if (count.count(i) > threshold)
        {
            high = i;
            break;
        }
    }

    if ((low = 0) and (high == 255))
    {
        return fb32::Image8880{input};
    }

    Image8880 output{d};

#if WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output, low, high](int start, int end)
    {
        rowsHistogramStretch(low, high, input, output, start, end);
    };

    tPool.detach_blocks<int>(0, d.height(), iterateRows);
    tPool.wait();
#else
    rowsHistogramStretch(low, high, input, output, 0, d.height());
#endif

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::maxRGB(
    const fb32::Interface8880Base& input)
{
    const auto d = input.getDimensions();
    Image8880 output{d};
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
    const fb32::Interface8880Base& input,
    fb32::Dimensions8880 d)
{
    if ((d.width() <= 0) or (d.height() <= 0))
    {
        throw std::invalid_argument("width and height must be greater than zero");
    }

    Image8880 output{d};
    resizeToBilinearInterpolation(input, output);

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::resizeLanczos3Interpolation(
    const fb32::Interface8880Base& input,
    fb32::Dimensions8880 d)
{
    if ((d.width() <= 0) or (d.height() <= 0))
    {
        throw std::invalid_argument("width and height must be greater than zero");
    }

    Image8880 output{d};
    resizeToLanczos3Interpolation(input, output);

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::resizeNearestNeighbour(
    const fb32::Interface8880Base& input,
    fb32::Dimensions8880 d)
{
    if ((d.width() <= 0) or (d.height() <= 0))
    {
        throw std::invalid_argument("width and height must be greater than zero");
    }

    Image8880 output{d};
    resizeToNearestNeighbour(input, output);

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::resizeToBilinearInterpolation(
    const fb32::Interface8880Base& input,
    fb32::Image8880& output)
{
    const auto od = output.getDimensions();
#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output](int start, int end)
    {
        rowsBilinearInterpolation(input, output, start, end);
    };

    tPool.detach_blocks<int>(0, od.height(), iterateRows);
    tPool.wait();
#else
    rowsBilinearInterpolation(input, output, 0, od.height());
#endif
    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::resizeToLanczos3Interpolation(
    const fb32::Interface8880Base& input,
    fb32::Image8880& output)
{
    const auto od = output.getDimensions();
#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output](int start, int end)
    {
        rowsLanczos3Interpolation(input, output, start, end);
    };

    tPool.detach_blocks<int>(0, od.height(), iterateRows);
    tPool.wait();
#else
    rowsLanczos3Interpolation(input, output, 0, od.height());
#endif
    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::resizeToNearestNeighbour(
    const fb32::Interface8880Base& input,
    fb32::Image8880& output)
{
    const auto od = output.getDimensions();
#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output](int start, int end)
    {
        rowsNearestNeighbour(input, output, start, end);
    };

    tPool.detach_blocks<int>(0, od.height(), iterateRows);
    tPool.wait();
#else
    rowsNearestNeighbour(input, output, 0, od.height());
#endif

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::rotate(
    const fb32::Interface8880Base& input,
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
        image = input;
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

    const auto id = image.getDimensions();

    const auto x10 = (id.width() * cosAngle) + (id.height() * sinAngle);
    const auto y00 = id.height() * cosAngle;
    const auto y11 = -(id.width() * sinAngle);

    const Dimensions8880 od{ static_cast<int>(std::ceil(x10)),
                             static_cast<int>(std::ceil(y00 - y11 + 1.0))};

    Image8880 output{od};
    output.clear(background);

#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&image, &output, sinAngle, cosAngle](int start, int end)
    {
        rowsRotate(image, output, sinAngle, cosAngle, start, end);
    };

    tPool.detach_blocks<int>(0, od.height(), iterateRows);
    tPool.wait();
#else
    rowsRotate(image, output, sinAngle, cosAngle, 0, od.height());
#endif

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::rotate90(
    const fb32::Interface8880Base& input)
{
    const auto id = input.getDimensions();
    const Dimensions8880 od{id.height(), id.width()};
    Image8880 output{od};

    for (auto j = 0 ; j < id.height() ; ++j)
    {
        for (auto i = 0 ; i < id.width() ; ++i)
        {
            const auto pixel{input.getPixel(Point{i, j})};

            if (not pixel.has_value())
            {
                continue;
            }

            output.setPixel(Point{id.height() - j - 1, i}, pixel.value());
        }
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::rotate180(
    const fb32::Interface8880Base& input)
{
    const auto d = input.getDimensions();
    Image8880 output{d};

    for (auto j = 0 ; j < d.height() ; ++j)
    {
        for (auto i = 0 ; i < d.width() ; ++i)
        {
            const auto pixel{input.getPixel(Point{i, j})};

            if (not pixel.has_value())
            {
                continue;
            }

            output.setPixel(Point{d.width() - i - 1, d.height() - j - 1}, pixel.value());
        }
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::rotate270(
    const fb32::Interface8880Base& input)
{
    const auto id = input.getDimensions();
    const Dimensions8880 od{ id.height(), id.width() };
    Image8880 output{od};

    for (auto j = 0 ; j < id.height() ; ++j)
    {
        for (auto i = 0 ; i < id.width() ; ++i)
        {
            const auto pixel{input.getPixel(Point{i, j})};

            if (not pixel.has_value())
            {
                continue;
            }

            output.setPixel(Point{j, id.width() - i - 1}, pixel.value());
        }
    }

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::scaleUp(
    const fb32::Interface8880Base& input,
    uint8_t scale)
{
    const auto id = input.getDimensions();
    const Dimensions8880 od { id.width() * scale, id.height() * scale };
    Image8880 output{od};

#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output, scale](int start, int end)
    {
        rowsScaleUp(input, output, scale, start, end);
    };

    tPool.detach_blocks<int>(0, id.height(), iterateRows);
    tPool.wait();
#else
    rowsScaleUp(input, output, scale, 0, id.height());
#endif

    return output;
}

//-------------------------------------------------------------------------

fb32::Image8880
fb32::toGrey(
    const Interface8880Base& input)
{
    const auto id = input.getDimensions();
    Image8880 output{id};

#ifdef WITH_BS_THREAD_POOL
    auto& tPool = threadPool();
    auto iterateRows = [&input, &output](int start, int end)
    {
        rowsToGrey(input, output, start, end);
    };

    tPool.detach_blocks<int>(0, id.height(), iterateRows);
    tPool.wait();
#else
    rowsToGrey(input, output, 0, id.height());
#endif

    return output;
}
