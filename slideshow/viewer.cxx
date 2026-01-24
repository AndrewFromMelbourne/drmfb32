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

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <ranges>

#include "image8880Font8x16.h"
#include "image8880Graphics.h"
#include "image8880Process.h"
#include "image8880Jpeg.h"
#include "viewer.h"

// ------------------------------------------------------------------------

namespace fs = std::filesystem;
using Point = fb32::Interface8880Point;

// ========================================================================

namespace
{

// ------------------------------------------------------------------------

std::string tolower(std::string_view s)
{
    std::string result;
    std::ranges::copy(std::views::transform(s, ::tolower), std::back_inserter(result));
    return result;
}

// ------------------------------------------------------------------------

}

// ========================================================================

Viewer::Quality
Viewer::qualityFromString(
    std::string_view string) noexcept
{
    auto s = tolower(string);

    if (s == "low")
    {
        return LOW;
    }

    if (s == "medium")
    {
        return MEDIUM;
    }

    if (s == "high")
    {
        return HIGH;
    }

    return MEDIUM;
}

// ------------------------------------------------------------------------

std::string
Viewer::qualityToString(
    Viewer::Quality quality) noexcept
{
    switch (quality)
    {
    case LOW:

        return "low";

    case MEDIUM:

        return "medium";

    case HIGH:

        return "high";
    }

    return "";
}

// ------------------------------------------------------------------------

Viewer::Viewer(
    fb32::Interface8880& interface,
    const std::string& folder,
    Viewer::Quality quality)
:
    m_annotate{true},
    m_buffer{interface.getWidth(), interface.getHeight()},
    m_current{INVALID_INDEX},
    m_directory{folder},
    m_enlighten{0},
    m_files{},
    m_fitToScreen{false},
    m_image{},
    m_imageProcessed{},
    m_offset{0, 0},
    m_percent{100},
    m_quality{quality},
    m_zoom{0}
{
    readDirectory();

    if (m_files.size() == 0)
    {
        throw std::invalid_argument("No files found.");
    }
}

// ------------------------------------------------------------------------

Viewer::~Viewer()
{
}

// ------------------------------------------------------------------------

void
Viewer::draw(
    fb32::FrameBuffer8880& fb) const
{
    fb.putImage(Point{0, 0}, m_buffer);
}

// ------------------------------------------------------------------------

bool
Viewer::update(
    fb32::Joystick& js)
{
    return handleImageViewing(js);
}

// ------------------------------------------------------------------------

void
Viewer::annotate()
{
    if (not m_annotate or not haveImages())
    {
        return;
    }

    auto name = m_files[m_current];
    auto annotation = name.substr(name.find_last_of('/') + 1);

    annotation += std::format(" ( {} x {} )",
                              m_image.getWidth(),
                              m_image.getHeight());

    annotation += std::format(" [ {} / {} ]", m_current + 1, m_files.size());
    annotation += std::format(" {}%", m_percent);

    annotation += std::format(" [ {} ]", qualityToString(m_quality));

    if (m_zoom)
    {
        annotation += std::format(" [ x{} ]", m_zoom);
    }
    else if (m_fitToScreen)
    {
        annotation += " [ FTS ]";
    }
    else
    {
        annotation += " [ FOS ]";
    }

    annotation += std::format(" [ enlighten {}% ]", m_enlighten * 10);

    fb32::Image8880Font8x16 font;
    constexpr int padding{4};
    const auto length = static_cast<int>(annotation.length());

    Point p1{0, 0};
    Point p2{length * font.getPixelWidth() + 2 * padding,
             font.getPixelHeight() + 2 * padding};

    constexpr fb32::RGB8880 black{0};
    constexpr fb32::RGB8880 green{0, 255, 0};

    fb32::boxFilled(m_buffer, p1, p2, black, 127);
    font.drawString(Point{padding, padding}, annotation, green, m_buffer);
}

// ------------------------------------------------------------------------

bool
Viewer::handleImageViewing(
    fb32::Joystick& js)
{
    if (js.buttonPressed(fb32::Joystick::BUTTON_Y))
    {
        imagePrevious();
        return true;
    }

    if (js.buttonPressed(fb32::Joystick::BUTTON_A))
    {
        imageNext();
        return true;
    }

    if (js.buttonPressed(fb32::Joystick::BUTTON_X))
    {
        if (m_zoom < MAX_ZOOM)
        {
            ++m_zoom;
            m_offset.zoomed(m_zoom);
            processImage();
            paint();
            return true;
        }
    }

    if (js.buttonPressed(fb32::Joystick::BUTTON_B))
    {
        if (m_zoom > 0)
        {
            --m_zoom;

            if (m_zoom == 0)
            {
                m_offset.center();
            }

            m_offset.zoomed(m_zoom);
            processImage();
            paint();
            return true;
        }
    }

    if (js.buttonPressed(fb32::Joystick::BUTTON_SELECT))
    {
        if (m_enlighten < 10)
        {
            ++m_enlighten;
        }
        else
        {
            m_enlighten = 0;
        }

        processImage();
        paint();
        return true;
    }

    if (js.buttonPressed(fb32::Joystick::BUTTON_LEFT_SHOULDER))
    {
            m_fitToScreen = !m_fitToScreen;
            processImage();
            paint();
            return true;
    }

    if (js.buttonPressed(fb32::Joystick::BUTTON_RIGHT_SHOULDER))
    {
        m_annotate = not m_annotate;
        paint();
        return true;
    }

    auto value = js.getAxes(0);

    if (not value.x and not value.y)
    {
        return false;
    }

    const auto dx = (value.x)
                  ? (10 * value.x / std::abs(value.x))
                  : 0;
    const auto dy = (value.y)
                  ? (10 * value.y / std::abs(value.y))
                  : 0;

    pan(dx, dy);
    paint();

    return true;
}

// ------------------------------------------------------------------------

void
Viewer::imageNext()
{
    if (haveImages())
    {
        ++m_current;

        if (m_current == m_files.size())
        {
            m_current = 0;
        }

        openImage();
    }
}

// ------------------------------------------------------------------------

void
Viewer::imagePrevious()
{
    if (haveImages())
    {
        if (m_current == 0)
        {
            m_current = m_files.size() - 1;
        }
        else
        {
            --m_current;
        }

        openImage();
    }
}

// ------------------------------------------------------------------------

void
Viewer::openImage()
{
    try
    {
        m_image = fb32::readJpeg(m_files[m_current]);
    }
    catch (std::invalid_argument& e)
    {
        std::println(std::cerr, "{} {}", m_files[m_current], e.what());
    }

    m_enlighten = 0;

    m_offset.center();

    processImage();
    paint();
}

// ------------------------------------------------------------------------

bool
Viewer::oversize() const noexcept
{
    if ((zoomedWidth() > m_buffer.getWidth()) or
        (zoomedHeight() > m_buffer.getHeight()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ------------------------------------------------------------------------

void
Viewer::paint()
{
    m_buffer.clear(0);

    if (not oversize())
    {
        m_offset.center();
    }

    m_buffer.putImage(placeImage(m_imageProcessed), m_imageProcessed);
    annotate();
}

// ------------------------------------------------------------------------

void
Viewer::pan(int x, int y) noexcept
{
    if (oversize() and (m_zoom != SCALE_OVERSIZED))
    {
        m_offset.pan(x, y, m_zoom);
    }
}

// ------------------------------------------------------------------------

fb32::Interface8880Point
Viewer::placeImage(
    const fb32::Image8880& image) const noexcept
{
    auto p = center(m_buffer, image);
    p.translate(m_offset.x(), m_offset.y());

    return p;
}

// ------------------------------------------------------------------------

void
Viewer::processImage()
{
    if (m_enlighten)
    {
        m_imageProcessed = enlighten(m_image, m_enlighten / 10.0);
    }
    else
    {
        m_imageProcessed = m_image;
    }

    if (((m_zoom == SCALE_OVERSIZED) and
         not oversize() and
         not m_fitToScreen) or (m_zoom == 1))
    {
        m_percent = 100;
    }
    else
    {
        if (m_zoom == SCALE_OVERSIZED)
        {
            int width = (m_buffer.getHeight() * m_image.getWidth()) /
                         m_image.getHeight();
            int height = m_buffer.getHeight();

            if (width > m_buffer.getWidth())
            {
                width = m_buffer.getWidth();
                height = (m_buffer.getWidth() * m_image.getHeight()) /
                          m_image.getWidth();
            }

            processResize(width, height);

            auto percent = (100.0 * m_imageProcessed.getWidth()) /
                           m_image.getWidth();
            m_percent = static_cast<int>(0.5 + percent);
        }
        else
        {
            m_imageProcessed = scaleUp(m_imageProcessed, m_zoom);
            m_percent = m_zoom * 100;
        }
    }
}

// ------------------------------------------------------------------------

void
Viewer::processResize(
    int width,
    int height)
{
    switch (m_quality)
    {
    case LOW:

        m_imageProcessed = resizeNearestNeighbour(m_imageProcessed, width, height);
        break;

    case MEDIUM:

        m_imageProcessed = resizeBilinearInterpolation(m_imageProcessed, width, height);
        break;

    case HIGH:

        m_imageProcessed = resizeLanczos3Interpolation(m_imageProcessed, width, height);
        break;
    }
}

// ------------------------------------------------------------------------

void
Viewer::readDirectory()
{
    m_files.clear();

    if (m_directory.length() > 0)
    {
        for (const auto& entry :
             fs::recursive_directory_iterator(m_directory))
        {
            auto ext = tolower(entry.path().extension().string());

            if (((ext == ".jpg") or (ext == ".jpeg")) and (entry.file_size() > 0))
            {
                m_files.push_back(entry.path().string());
            }
        }
    }

    if (m_files.size() > 0)
    {
        std::ranges::sort(m_files);

        m_current = 0;
        openImage();
    }
    else
    {
        m_current = INVALID_INDEX;
        m_offset.center();
    }
}

// ------------------------------------------------------------------------

int
Viewer::zoomedHeight() const noexcept
{
    const auto zoom = (m_zoom == 0) ? 1 : m_zoom;

    return m_image.getHeight() * zoom;
}

// ------------------------------------------------------------------------

int
Viewer::zoomedWidth() const noexcept
{
    const auto zoom = (m_zoom == 0) ? 1 : m_zoom;

    return m_image.getWidth() * zoom;
}

