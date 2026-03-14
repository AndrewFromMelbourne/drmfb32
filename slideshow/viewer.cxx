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
#include <array>
#include <cctype>
#include <filesystem>
#include <format>
#include <iostream>
#include <iterator>
#include <map>
#include <print>
#include <ranges>
#include <span>

#include "image8880Font8x16.h"
#include "image8880Graphics.h"
#include "image8880Process.h"
#include "image8880Jpeg.h"
#include "image8880Png.h"
#include "image8880Qoi.h"
#include "viewer.h"

//-------------------------------------------------------------------------

namespace fs = std::filesystem;
using Point = fb32::Point8880;
using MenuItem = fb32::Interface8880Menu::MenuItem;

// ========================================================================

namespace
{

//-------------------------------------------------------------------------

[[nodiscard]] std::vector<std::string>
annotateStrings()
{
    std::vector<std::string> result;

    for (const auto annotate : { Viewer::ANNOTATE_OFF, Viewer::ANNOTATE_SHORT, Viewer::ANNOTATE_LONG })
    {
        result.push_back(Viewer::annotateToString(annotate));
    }

    return result;
}


//-------------------------------------------------------------------------

std::string
    tolower(
        std::string_view s)
{
    std::string result;
    std::ranges::copy(std::views::transform(s, ::tolower), std::back_inserter(result));
    return result;
}

//-------------------------------------------------------------------------

[[nodiscard]] bool
isImageFile(
    std::string_view ext)
{
    static constexpr const char* extensions[] = {
        ".jpg",
        ".jpeg",
        ".png",
        ".qoi"
    };

    return std::ranges::find(extensions, ext) != std::end(extensions);
}

//-------------------------------------------------------------------------

[[nodiscard]] std::vector<std::string>
boolStrings()
{
    return std::vector<std::string>{ "no", "yes" };
}

//-------------------------------------------------------------------------

[[nodiscard]] std::span<const int>
fileStep() noexcept
{
    static std::array steps{ 1, 2, 5, 10, 20, 50, 100 };

    return steps;
}


//-------------------------------------------------------------------------

[[nodiscard]] std::vector<std::string>
fileStepStrings() noexcept
{
    std::vector<std::string> result;
    const auto fs = fileStep();
    result.reserve(fs.size());
    std::transform(cbegin(fs),
                   cend(fs),
                   std::back_inserter(result),
                   [](int i){ return std::to_string(i); });
    return result;
}

//-------------------------------------------------------------------------

[[nodiscard]] std::span<const int>
panStep() noexcept
{
    static std::array steps{ 1, 2, 5, 10, 20, 50, 100 };

    return steps;
}


//-------------------------------------------------------------------------

[[nodiscard]] std::vector<std::string>
panStepStrings() noexcept
{
    std::vector<std::string> result;
    const auto ps = panStep();
    result.reserve(ps.size());
    std::transform(cbegin(ps),
                   cend(ps),
                   std::back_inserter(result),
                   [](int i){ return std::to_string(i); });
    return result;
}

//-------------------------------------------------------------------------

[[nodiscard]] std::vector<std::string>
percentageStrings(
    int step)
{
    std::vector<std::string> result;

    for (int percent = 0 ; percent <= 100 ; percent += step)
    {
        result.push_back(std::format("{}%", percent));
    }

    return result;
}

//-------------------------------------------------------------------------

[[nodiscard]] std::vector<std::string>
qualityStrings()
{
    std::vector<std::string> result;

    for (const auto quality : { Viewer::QUALITY_LOW, Viewer::QUALITY_MEDIUM, Viewer::QUALITY_HIGH })
    {
        result.push_back(Viewer::qualityToString(quality));
    }

    return result;
}

//-------------------------------------------------------------------------

[[nodiscard]] std::vector<std::string>
zoomStrings(
    int maximum)
{
    std::vector<std::string> result{"fit"};

    for (int zoom = 1 ; zoom <= maximum ; ++zoom)
    {
        result.push_back(std::format("{}x", zoom));
    }

    return result;
}

//-------------------------------------------------------------------------

} // namespace

// ========================================================================

Viewer::Annotate
Viewer::annotateFromString(
    std::string_view string) noexcept
{
    auto s = tolower(string);

    if (s == "off")
    {
        return ANNOTATE_OFF;
    }

    if (s == "medium")
    {
        return ANNOTATE_SHORT;
    }

    if (s == "high")
    {
        return ANNOTATE_LONG;
    }

    return ANNOTATE_SHORT;
}

//-------------------------------------------------------------------------

std::string
Viewer::annotateToString(
    Viewer::Annotate annotate) noexcept
{
    switch (annotate)
    {
    case ANNOTATE_OFF:

        return "off";

    case ANNOTATE_SHORT:

        return "short";

    case ANNOTATE_LONG:

        return "long";
    }

    return "";
}

//-------------------------------------------------------------------------

Viewer::Quality
Viewer::qualityFromString(
    std::string_view string) noexcept
{
    auto s = tolower(string);

    if (s == "low")
    {
        return QUALITY_LOW;
    }

    if (s == "medium")
    {
        return QUALITY_MEDIUM;
    }

    if (s == "high")
    {
        return QUALITY_HIGH;
    }

    return QUALITY_MEDIUM;
}

//-------------------------------------------------------------------------

std::string
Viewer::qualityToString(
    Viewer::Quality quality) noexcept
{
    switch (quality)
    {
    case QUALITY_LOW:

        return "low";

    case QUALITY_MEDIUM:

        return "medium";

    case QUALITY_HIGH:

        return "high";
    }

    return "";
}

//-------------------------------------------------------------------------

Viewer::Viewer(
    fb32::RGB8880 background,
    fb32::Interface8880& interface,
    const std::string& folder,
    Viewer::Quality quality,
    const fb32::FontConfig& fontConfig)
:
    m_annotate{ANNOTATE_SHORT},
    m_background{background},
    m_buffer{interface.getDimensions()},
    m_current{INVALID_INDEX},
    m_directory{folder},
    m_enlighten{0},
    m_extToType{
        {".jpg", Type::JPEG},
        {".jpeg", Type::JPEG},
        {".png", Type::PNG},
        {".qoi", Type::QOI}
    },
    m_files{},
    m_fileStep{1},
    m_fitToScreen{true},
    m_font{createFont(fontConfig)},
    m_image{},
    m_imageProcessed{},
    m_isBlank{false},
    m_menu{
        fb32::RGB8880{0x00FFFFFF},
        fb32::RGB8880{0x00000000},
        fb32::RGB8880{0x003F3F3F},
        fontConfig,
        {
            MenuItem{MENUID_ANNOTATE, "Annotate", ANNOTATE_SHORT, annotateStrings()},
            MenuItem{MENUID_ENLIGHTEN, "Enlighten", 0, percentageStrings(10)},
            MenuItem{MENUID_FILE_STEP, "File step", 1, fileStepStrings()},
            MenuItem{MENUID_FIT_TO_SCREEN, "Fit to screen", 1, boolStrings()},
            MenuItem{MENUID_PAN_STEP, "Pan step", 3, panStepStrings()},
            MenuItem{MENUID_QUALITY, "Quality", quality, qualityStrings()},
            MenuItem{MENUID_ZOOM, "Zoom", 0, zoomStrings(MAX_ZOOM)}
        }
    },
    m_menuShow{false},
    m_offset{0, 0},
    m_panStep{10},
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

//-------------------------------------------------------------------------

Viewer::~Viewer()
{
}

//-------------------------------------------------------------------------

void
Viewer::draw(
    fb32::FrameBuffer8880& fb) const
{
    fb.putImage(Point{0, 0}, m_buffer);

    if (m_menuShow)
    {
        m_menu.draw(fb);
    }
}

//-------------------------------------------------------------------------

bool
Viewer::update(
    fb32::Joystick& js)
{
    if (js.buttonPressed(fb32::Joystick::BUTTON_RIGHT_SHOULDER) and not m_menuShow)
    {
        m_isBlank = not m_isBlank;
        js.buttonsClear();
        paint();
        return true;
    }

    if (m_isBlank)
    {
        return false;
    }

    if (js.buttonPressed(fb32::Joystick::BUTTON_SELECT))
    {
        m_menuShow = not m_menuShow;
        js.buttonsClear();

        if (m_menuShow)
        {
            setMenuValues();
        }

        return true;
    }

    if (m_menuShow)
    {
        const auto updated = m_menu.update(js);

        switch (updated)
        {
        case fb32::Interface8880Menu::NO_UPDATE:

            return false;
            break;

        case fb32::Interface8880Menu::MENU_UPDATE:

            return true;
            break;

        case fb32::Interface8880Menu::VALUE_UPDATE:

            readValuesFromMenu();
            processImage();
            paint();
            return true;
            break;
        }
    }

    return handleImageViewing(js);
}

//-------------------------------------------------------------------------

void
Viewer::annotate()
{
    if (m_annotate == ANNOTATE_OFF or not haveImages())
    {
        return;
    }

    auto [name, type] = m_files[m_current];
    auto annotation = fs::path(name).filename().string();
    const auto d = m_image.getDimensions();

    annotation += std::format(" ({}x{})", d.width(), d.height());
    annotation += std::format(" [{}/{}]", m_current + 1, m_files.size());

    if (m_annotate == ANNOTATE_LONG)
    {
        annotation += std::format(" {}%", m_percent);
        annotation += std::format(" [{}]", qualityToString(m_quality));
    }

    if (m_zoom)
    {
        annotation += std::format(" [x{}]", m_zoom);
    }
    else if (m_fitToScreen)
    {
        annotation += " [FTS]";
    }
    else
    {
        annotation += " [FOS]";
    }

    if (m_annotate == ANNOTATE_LONG)
    {
        annotation += std::format(" [enlighten {}%]", m_enlighten * 10);
    }

    constexpr int padding{4};
    constexpr auto padding2(2 * padding);
    const auto dimensions = m_font->getStringDimensions(annotation);

    Point p1{0, 0};
    Point p2{dimensions.width() + padding2, dimensions.height() + padding2};

    constexpr fb32::RGB8880 black{0};
    constexpr fb32::RGB8880 green{0, 255, 0};

    fb32::boxFilled(m_buffer, p1, p2, black, 127);
    m_font->drawString(Point{padding, padding}, annotation, green, m_buffer);
}

//-------------------------------------------------------------------------

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

    const auto value = js.getAxes(0);

    if (not value.x and not value.y)
    {
        return false;
    }

    const auto dx = (value.x)
                  ? (m_panStep * value.x / std::abs(value.x))
                  : 0;
    const auto dy = (value.y)
                  ? (m_panStep * value.y / std::abs(value.y))
                  : 0;

    pan(dx, dy);
    paint();

    return true;
}

//-------------------------------------------------------------------------

void
Viewer::imageNext()
{
    if (haveImages())
    {
        m_current += m_fileStep;

        if (m_current >= m_files.size())
        {
            m_current = 0;
        }

        openImage();
    }
}

//-------------------------------------------------------------------------

void
Viewer::imagePrevious()
{
    if (haveImages())
    {
        if (static_cast<int>(m_current) <= m_fileStep)
        {
            m_current = m_files.size() - 1;
        }
        else
        {
            m_current -= m_fileStep;
        }

        openImage();
    }
}

//-------------------------------------------------------------------------

void
Viewer::openImage()
{
    auto [name, type] = m_files[m_current];

    try
    {
        switch (type)
        {
        case Type::JPEG:
            m_image = fb32::readJpeg(name);
            break;
        case Type::PNG:
            m_image = fb32::readPng(name, m_background);
            break;
        case Type::QOI:
            m_image = fb32::readQoi(name, m_background);
            break;
        }
    }
    catch (std::invalid_argument& e)
    {
        std::println(std::cerr, "{} {}", name, e.what());
    }

    m_enlighten = 0;

    m_offset.center();

    processImage();
    paint();
}

//-------------------------------------------------------------------------

bool
Viewer::oversize() const noexcept
{
    const auto d = m_buffer.getDimensions();
    const auto zd = zoomedDimensions();

    if ((zd.width() > d.width()) or (zd.height() > d.height()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//-------------------------------------------------------------------------

void
Viewer::paint()
{
    m_buffer.clear(m_background);

    if (m_isBlank)
    {
        return;
    }

    if (not oversize())
    {
        m_offset.center();
    }

    m_buffer.putImage(placeImage(m_imageProcessed), m_imageProcessed);
    annotate();
}

//-------------------------------------------------------------------------

void
Viewer::pan(int x, int y) noexcept
{
    if (oversize() and (m_zoom != SCALE_OVERSIZED))
    {
        m_offset.pan(x, y, m_zoom);
    }
}

//-------------------------------------------------------------------------

fb32::Point8880
Viewer::placeImage(
    const fb32::Image8880& image) const noexcept
{
    auto p = center(m_buffer, image);
    p.translate(m_offset.x(), m_offset.y());

    return p;
}

//-------------------------------------------------------------------------

void
Viewer::processImage()
{
    const auto id = m_image.getDimensions();

    if (id.width() == 0 or id.height() == 0)
    {
        m_imageProcessed = m_image;
        return;
    }

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
            const auto bd = m_buffer.getDimensions();
            fb32::Dimensions8880 d
            {
                (bd.height() * id.width()) / id.height(),
                bd.height()
            };

            if (d.width() > bd.width())
            {
                d.set(
                    bd.width(),
                    (bd.width() * id.height()) / id.width());
            }

            processResize(d);

            auto percent = (100.0 * m_imageProcessed.getDimensions().width()) / id.width();
            m_percent = static_cast<int>(0.5 + percent);
        }
        else
        {
            m_imageProcessed = scaleUp(m_imageProcessed, m_zoom);
            m_percent = m_zoom * 100;
        }
    }
}

//-------------------------------------------------------------------------

void
Viewer::processResize(
    fb32::Dimensions8880 d)
{
    switch (m_quality)
    {
    case QUALITY_LOW:

        m_imageProcessed = resizeNearestNeighbour(m_imageProcessed, d);
        break;

    case QUALITY_MEDIUM:

        m_imageProcessed = resizeBilinearInterpolation(m_imageProcessed, d);
        break;

    case QUALITY_HIGH:

        m_imageProcessed = resizeLanczos3Interpolation(m_imageProcessed, d);
        break;
    }
}

//-------------------------------------------------------------------------

void
Viewer::readDirectory()
{
    m_files.clear();

    if (m_directory.length() > 0)
    {
        for (const auto& entry :
             fs::recursive_directory_iterator(m_directory))
        {
            if (not entry.is_regular_file())
            {
                continue;
            }

            auto ext = tolower(entry.path().extension().string());

            if ((entry.file_size() > 0) and isImageFile(ext))
            {
                m_files.emplace_back(
                    entry.path().string(),
                    m_extToType.at(ext));
            }
        }
    }

    if (m_files.size() > 0)
    {
        std::sort(begin(m_files), end(m_files));

        m_current = 0;
        openImage();
    }
    else
    {
        m_current = INVALID_INDEX;
        m_offset.center();
    }
}

//-------------------------------------------------------------------------

void
Viewer::readValuesFromMenu()
{
    m_annotate = static_cast<Annotate>(m_menu.getValue(MENUID_ANNOTATE));
    m_enlighten = m_menu.getValue(MENUID_ENLIGHTEN);
    m_fitToScreen = m_menu.getValue(MENUID_FIT_TO_SCREEN);
    m_quality = static_cast<Quality>(m_menu.getValue(MENUID_QUALITY));
    m_zoom = m_menu.getValue(MENUID_ZOOM);

    const auto panStepIndex = m_menu.getValue(MENUID_PAN_STEP);
    const auto panSteps = panStep();
    m_panStep =  panSteps[panStepIndex];

    const auto fileStepIndex = m_menu.getValue(MENUID_FILE_STEP);
    const auto fileSteps = fileStep();
    m_fileStep = fileSteps[fileStepIndex];
}

//-------------------------------------------------------------------------

void
Viewer::setMenuValues()
{
    m_menu.setValue(MENUID_ANNOTATE, m_annotate);
    m_menu.setValue(MENUID_ENLIGHTEN, m_enlighten);
    m_menu.setValue(MENUID_FIT_TO_SCREEN, m_fitToScreen);
    m_menu.setValue(MENUID_QUALITY, m_quality);
    m_menu.setValue(MENUID_ZOOM, m_zoom);

    std::size_t index = 0UL;
    for (const auto step : panStep())
    {
        if (static_cast<int>(step) == m_panStep)
        {
            m_menu.setValue(MENUID_PAN_STEP, index);
            break;
        }
        ++index;
    }

    index = 0UL;
    for (const auto step : fileStep())
    {
        if (static_cast<int>(step) == m_fileStep)
        {
            m_menu.setValue(MENUID_FILE_STEP, index);
            break;
        }
        ++index;
    }
}

//-------------------------------------------------------------------------

fb32::Dimensions8880
Viewer::zoomedDimensions() const noexcept
{
    const auto d = m_image.getDimensions();
    const auto zoom = (m_zoom == 0) ? 1 : m_zoom;

    return {d.width() * zoom, d.height() * zoom};
}

