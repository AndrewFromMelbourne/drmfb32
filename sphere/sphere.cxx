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
#include <array>
#include <cmath>
#include <cstring>
#include <functional>

#include "sphere.h"

//-------------------------------------------------------------------------

using namespace fb32;

//-------------------------------------------------------------------------

double dot(vector3 a, vector3 b) noexcept
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

//-------------------------------------------------------------------------

Sphere::Sphere(int size)
:
    m_size{size},
    m_image(size, size),
    m_ambient{ 0.3 },
#ifdef WITH_BS_THREAD_POOL
    m_light{ -std::sqrt(1.0/3.0), std::sqrt(1.0/3.0), std::sqrt(1.0/3.0) },
    m_threadPool()
#else
    m_light{ -std::sqrt(1.0/3.0), std::sqrt(1.0/3.0), std::sqrt(1.0/3.0) }
    m_image(size, size)
#endif
{
}

//-------------------------------------------------------------------------

void
Sphere::init()
{
    m_image.clear(0);
}

//-------------------------------------------------------------------------

void
Sphere::update()
{
#ifdef WITH_BS_THREAD_POOL
    auto iterateRows = [this](int start, int end)
    {
        updateRows(start, end);
    };

    m_threadPool.detach_blocks<int>(0, m_size, iterateRows);
    m_threadPool.wait();
#else
    updateRows(0, m_size);
#endif
}

//-------------------------------------------------------------------------

void
Sphere::updateRows(
    int jStart,
    int jEnd)
{
    const auto radius = m_size / 2;

    for (auto j = jStart; j < jEnd; ++j)
    {
        const double y = double(radius - j) / radius;
        for (auto i = 0 ; i < m_size ; ++i)
        {
            uint8_t grey{};

            const double x = double(i - radius) / radius;
            const double sum = x * x + y * y;

            if (sum <= 1.0)
            {
                const double z = std::sqrt(1.0 - sum);
                const vector3 v{x, y, z};
                double intensity = dot(v, m_light);

                intensity = std::clamp(intensity, 0.0, 1.0);

                intensity *= intensity;
                intensity *= (1.0 - m_ambient);

                grey = (uint8_t)std::ceil(200.0 * (intensity + m_ambient));
            }

            const Interface8880Point p{i, j};
            m_image.setPixelRGB(p, RGB8880(grey, grey, grey));
        }
    }
}

//-------------------------------------------------------------------------

void
Sphere::setLight(
    double inclination,
    double bearing) noexcept
{
    inclination = std::clamp(inclination, 0.0, 90.0);

    auto deg2rad = [](double deg) -> double
    {
        return (deg * M_PI) / 180.0;
    };

    const auto radius{cos(deg2rad(inclination))};

    m_light[0] = radius * sin(deg2rad(bearing)); // X
    m_light[1] = radius * cos(deg2rad(bearing)); // Y
    m_light[2] = sin(deg2rad(inclination)); // Z
}

//-------------------------------------------------------------------------

void
Sphere::draw(
    fb32::FrameBuffer8880& fb)
{
    fb.putImage(center(fb, m_image), m_image);
}

