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

#include <limits>
#include <map>
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

    // --------------------------------------------------------------------

    enum Quality
    {
        LOW,
        MEDIUM,
        HIGH
    };

    enum Type
    {
        JPEG,
        PNG,
        QOI
    };

    // --------------------------------------------------------------------

    struct ImageFile
    {
        std::string m_filename;
        Type m_type;

        auto operator<(const ImageFile& other) const
        {
            return m_filename < other.m_filename;
        }
    };


    // --------------------------------------------------------------------

    [[nodiscard]] static Quality qualityFromString(std::string_view string) noexcept;
    [[nodiscard]] static std::string qualityToString(Quality quality) noexcept;

    // --------------------------------------------------------------------

    class Offset
    {
    public:

        Offset(int x, int y) noexcept
        :
            m_x{x},
            m_y{y},
            m_zoom{1},
            m_zoomedX{x},
            m_zoomedY{y}
        {}

        void center() noexcept
        {
            m_x = 0;
            m_y = 0;
            m_zoomedX = 0;
            m_zoomedY = 0;
        }

        void pan(int dx, int dy, int zoom) noexcept
        {
            m_x += dx;
            m_y += dy;
            zoomed(zoom);
        }

        [[nodiscard]] int x() const noexcept { return m_zoomedX; }
        [[nodiscard]] int y() const noexcept { return m_zoomedY; }

        void zoomed(int zoom) noexcept
        {
            m_zoom = (zoom == 0) ? 1 : zoom;
            m_zoomedX = m_x * m_zoom;
            m_zoomedY = m_y * m_zoom;
        }

    private:

        int m_x{};
        int m_y{};
        int m_zoom{1};
        int m_zoomedX{};
        int m_zoomedY{};
    };

    // --------------------------------------------------------------------

    Viewer(
        fb32::RGB8880 background,
        fb32::Interface8880& interface,
        const std::string& folder,
        Quality quality);

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
    void processResize(int width, int height);
    void readDirectory();
    [[nodiscard]] int zoomedHeight() const noexcept;
    [[nodiscard]] int zoomedWidth() const noexcept;

    static const std::size_t INVALID_INDEX{std::numeric_limits<std::size_t>::max()};
    static const int MAX_ZOOM{5};
    static const int SCALE_OVERSIZED{0};

    bool m_annotate;
    fb32::RGB8880 m_background;
    fb32::Image8880 m_buffer;
    std::size_t m_current;
    std::string m_directory;
    int m_enlighten;
    std::map<std::string, Type> m_extToType;
    std::vector<ImageFile> m_files;
    bool m_fitToScreen;
    fb32::Image8880 m_image;
    fb32::Image8880 m_imageProcessed;
    Offset m_offset;
    int m_percent;
    Quality m_quality;
    int m_zoom;
};
