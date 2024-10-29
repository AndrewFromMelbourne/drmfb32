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
#include <iostream>

#include "image8880Font8x16.h"
#include "image8880Jpeg.h"
#include "viewer.h"

// ------------------------------------------------------------------------

namespace fs = std::filesystem;

// ========================================================================

namespace
{

// ------------------------------------------------------------------------

std::string tolower(std::string s)
{
    std::transform(s.begin(),
                   s.end(),
                   s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

// ------------------------------------------------------------------------

}
// ========================================================================


Viewer::Viewer(
    fb32::Interface8880& interface,
    const std::string& folder)
:
    m_annotate{true},
    m_buffer{interface.getWidth(), interface.getHeight()},
    m_current{-1},
    m_directory{folder},
    m_files{},
    m_fitToScreen{false},
    m_image{},
    m_imageProcessed{},
    m_isBlank{false},
    m_percent{100},
    m_xOffset{0},
    m_yOffset{0},
    m_zoom{0}
{
    readDirectory();
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
    fb.putImage(fb32::Interface8880Point{0, 0}, m_buffer);
}

// ------------------------------------------------------------------------

bool
Viewer::update(
    fb32::Joystick& js)
{
    auto result = handleGeneral(js);

    if (viewingImage() and handleImageViewing(js))
    {
        result = true;
    }

    return result;
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
    auto annotation = name.substr(m_directory.length());

    annotation += " ( " +
                  std::to_string(m_image.getWidth()) +
                  " x " +
                  std::to_string(m_image.getHeight()) +
                  " )";

    annotation += " [ " +
                  std::to_string(m_current + 1) +
                  " / " +
                  std::to_string(m_files.size()) +
                  " ]";

    annotation += " " + std::to_string(m_percent) + "%";

    if (m_zoom)
    {
        annotation += " [ x" + std::to_string(m_zoom) + " ]";
    }
    else if (m_fitToScreen)
    {
        annotation += " [ FTS ]";
    }
    else
    {
        annotation += " [ FOS ]";
    }

    fb32::Image8880Font8x16 font;
    font.drawString(
        fb32::Interface8880Point{0, 0},
        annotation,
        fb32::RGB8880{0, 255, 0},
        m_buffer);
}

// ------------------------------------------------------------------------

bool
Viewer::handleGeneral(
    fb32::Joystick& js)
{
    if (js.buttonPressed(fb32::Joystick::BUTTON_SELECT))
    {
        m_isBlank = not m_isBlank;
        paint();
        return true;
    }

    return false;
}

// ------------------------------------------------------------------------

bool
Viewer::handleImageViewing(
    fb32::Joystick& js)
{
    bool result{false};

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
                m_xOffset = 0;
                m_yOffset = 0;
            }

            processImage();
            paint();
            return true;
        }
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

    auto dx = (value.x) ? (10 * value.x / std::abs(value.x)) : 0;
    auto dy = (value.y) ? (10 * value.y / std::abs(value.y)) : 0;

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
        --m_current;

        if (m_current == -1)
        {
            m_current = m_files.size() - 1;
        }

        openImage();
    }
}

// ------------------------------------------------------------------------

void
Viewer::openImage()
{
    m_image = fb32::readJpeg(m_files[m_current]);

    m_xOffset = 0;
    m_yOffset = 0;

    processImage();
    paint();
}

// ------------------------------------------------------------------------

bool
Viewer::oversize() const
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

    if (m_isBlank)
    {
        return;
    }

    if (not oversize())
    {
        m_xOffset = 0;
        m_yOffset = 0;
    }

    m_buffer.putImage(placeImage(m_imageProcessed), m_imageProcessed);
    annotate();
}

// ------------------------------------------------------------------------

void
Viewer::pan(int x, int y)
{
    if (oversize() and (m_zoom != SCALE_OVERSIZED))
    {
        m_xOffset += (x * m_zoom);
        m_yOffset += (y * m_zoom);
    }
}

// ------------------------------------------------------------------------

fb32::Interface8880Point
Viewer::placeImage(
    const fb32::Image8880& image) const
{
    auto x = (m_buffer.getWidth() / 2) - (image.getWidth() / 2) + m_xOffset;
    auto y = (m_buffer.getHeight() / 2) - (image.getHeight() / 2) + m_yOffset;

    return fb32::Interface8880Point{x, y};
}

// ------------------------------------------------------------------------

void
Viewer::processImage()
{
    if (((m_zoom == SCALE_OVERSIZED) and
         not oversize() and
         not m_fitToScreen) or (m_zoom == 1))
    {
        m_imageProcessed = m_image;
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

            m_imageProcessed = m_image.resizeNearestNeighbour(width, height);
            auto percent = (100.0 * m_imageProcessed.getWidth()) /
                           m_image.getWidth();
            m_percent = static_cast<int>(0.5 + percent);
        }
        else
        {
            m_imageProcessed = m_image.scaleUp(m_zoom);
            m_percent = m_zoom * 100;
        }
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
        std::sort(m_files.begin(), m_files.end());

        m_current = 0;
        openImage();
    }
    else
    {
        m_current = INVALID_INDEX;

        m_xOffset = 0;
        m_yOffset = 0;
    }
}

// ------------------------------------------------------------------------

int
Viewer::zoomedHeight() const
{
    auto zoom = (m_zoom == 0) ? 1 : m_zoom;

    return m_image.getHeight() * zoom;
}

// ------------------------------------------------------------------------

int
Viewer::zoomedWidth() const
{
    auto zoom = (m_zoom == 0) ? 1 : m_zoom;

    return m_image.getWidth() * zoom;
}

