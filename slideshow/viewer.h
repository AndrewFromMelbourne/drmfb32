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

#pragma once

// ------------------------------------------------------------------------

#include <string>
#include <vector>

#include "framebuffer8880.h"
#include "image8880.h"
#include "interface8880.h"
#include "joystick.h"

// ------------------------------------------------------------------------

class Viewer
{
public:

    Viewer(fb32::Interface8880& interface, const std::string& folder);
    ~Viewer();

    Viewer(const Viewer&) = delete;
    Viewer& operator=(const Viewer&) = delete;

    Viewer(Viewer&& image) = delete;
    Viewer& operator=(Viewer&& image) = delete;

    void draw(fb32::FrameBuffer8880& fb) const;
    bool update(fb32::Joystick& js);

private:

    [[nodiscard]] bool haveImages() const noexcept { return m_current != INVALID_INDEX; }
    [[nodiscard]] bool originalSize() const noexcept { return m_percent == 100; }

    void annotate();
    bool handleImageViewing(fb32::Joystick& js);
    void imageNext();
    void imagePrevious();
    void openImage();
    [[nodiscard]] bool oversize() const noexcept;
    void paint();
    void pan(int dx, int dy) noexcept;
    [[nodiscard]] fb32::Interface8880Point placeImage(const fb32::Image8880& image) const noexcept;
    void processImage();
    void readDirectory();
    [[nodiscard]] int zoomedHeight() const noexcept;
    [[nodiscard]] int zoomedWidth() const noexcept;

    static const int INVALID_INDEX{-1};
    static const int MAX_ZOOM{5};
    static const int SCALE_OVERSIZED{0};

    bool m_annotate;
    fb32::Image8880 m_buffer;
    int m_current;
    std::string m_directory;
    int m_enlighten;
    std::vector<std::string> m_files;
    bool m_fitToScreen;
    fb32::Image8880 m_image;
    fb32::Image8880 m_imageProcessed;
    int m_percent;
    int m_xOffset;
    int m_yOffset;
    int m_zoom;
};
