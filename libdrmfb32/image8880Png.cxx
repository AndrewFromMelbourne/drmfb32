//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2026 Andrew Duncan
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

#include "image8880.h"
#include "image8880Png.h"

//-------------------------------------------------------------------------

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include <png.h>

//=========================================================================

namespace
{

//-------------------------------------------------------------------------

class PngDecode
{
public:

    explicit
    PngDecode(
        std::span<const uint8_t> data,
        const fb32::RGB8880& background);

    ~PngDecode();

    PngDecode(const PngDecode& fb) = delete;
    PngDecode& operator=(const PngDecode& fb) = delete;

    PngDecode(PngDecode&& fb) = delete;
    PngDecode& operator=(PngDecode&& fb) = delete;

    void decodeIntoImage(fb32::Image8880& image);
    fb32::Image8880 decode();

    static void userErrorFn(png_structp, png_const_charp errorMsg)
    {
        throw std::runtime_error{errorMsg};
    }

    static void userWarningFn(png_structp, png_const_charp)
    {
        // Ignore warnings
    }

private:

    void pngStartRead();

    fb32::RGB8880 m_background;
    std::span<const uint8_t> m_data;
    png_structp m_readPtr;
    png_infop m_infoPtr;
};

//-------------------------------------------------------------------------

void pngReadDataCallback(
    png_structp pngPtr,
    png_bytep outBytes,
    png_size_t byteCountToRead)
{
    auto* const data{static_cast<std::span<const uint8_t>*>(png_get_io_ptr(pngPtr))};

    if (byteCountToRead > data->size())
    {
        png_error(pngPtr, "Read Error");
    }

    std::copy(data->begin(), data->begin() + byteCountToRead, outBytes);
    *data = data->subspan(byteCountToRead);
}

//-------------------------------------------------------------------------

PngDecode::PngDecode(
    std::span<const uint8_t> data,
    const fb32::RGB8880& background)
:
    m_background{background},
    m_data{data},
    m_readPtr{png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                     reinterpret_cast<png_voidp>(this),
                                     userErrorFn,
                                     userWarningFn)},
    m_infoPtr{png_create_info_struct(m_readPtr)}
{
    try
    {
        if (not m_readPtr or not m_infoPtr)
        {
            return;
        }

        pngStartRead();
    }
    catch (std::exception&)
    {
        png_destroy_read_struct(&m_readPtr, &m_infoPtr, nullptr);
        m_readPtr = nullptr;
        m_infoPtr = nullptr;
    }
}

//-------------------------------------------------------------------------

PngDecode::~PngDecode()
{
    png_destroy_read_struct(&m_readPtr, &m_infoPtr, nullptr);
}

//-------------------------------------------------------------------------

void
PngDecode::pngStartRead()
{
    png_set_read_fn(m_readPtr, &m_data, pngReadDataCallback);
    png_read_info(m_readPtr, m_infoPtr);

    double gamma{0.0};
    if (png_get_gAMA(m_readPtr, m_infoPtr, &gamma))
    {
        png_set_gamma(m_readPtr, 2.2, gamma);
    }

    png_set_expand(m_readPtr);
#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
    png_set_scale_16(m_readPtr);
#else
    png_set_strip_16(m_readPtr);
#endif
    png_set_packing(m_readPtr);

    const int colorType{png_get_color_type(m_readPtr, m_infoPtr)};
    if (colorType == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(m_readPtr);
    }

    if (png_get_valid(m_readPtr, m_infoPtr, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(m_readPtr);
    }

    if (colorType == PNG_COLOR_TYPE_GRAY and
        (png_get_bit_depth(m_readPtr, m_infoPtr) < 8))
    {
        png_set_expand_gray_1_2_4_to_8(m_readPtr);
    }

    if ((colorType == PNG_COLOR_TYPE_GRAY) or
        (colorType == PNG_COLOR_TYPE_GRAY_ALPHA))
    {
        png_set_gray_to_rgb(m_readPtr);
    }

    png_color_16 *imageBackgroundColor{};

    if (png_get_bKGD(m_readPtr, m_infoPtr, &imageBackgroundColor))
    {
        png_set_background(m_readPtr,
                           imageBackgroundColor,
                           PNG_BACKGROUND_GAMMA_FILE,
                           1,
                           1.0);
    }
    else
    {
        png_color_16 background = {
            .index = 0,
            .red = m_background.getRed(),
            .green = m_background.getGreen(),
            .blue = m_background.getBlue(),
            .gray = 0
        };

        png_set_background(m_readPtr,
                           &background,
                           PNG_BACKGROUND_GAMMA_SCREEN,
                           0,
                           1.0);
    }

    png_set_filler(m_readPtr, 0xFF, PNG_FILLER_AFTER);
    png_set_bgr(m_readPtr);

    png_read_update_info(m_readPtr, m_infoPtr);
}

//-------------------------------------------------------------------------

void
PngDecode::decodeIntoImage(
    fb32::Image8880& image)
{
    if (not m_readPtr or not m_infoPtr)
    {
        return;
    }

    try
    {
        const auto width = png_get_image_width(m_readPtr, m_infoPtr);
        const auto height = png_get_image_height(m_readPtr, m_infoPtr);

        if ((image.getWidth() != static_cast<int>(width)) or
            (image.getHeight() != static_cast<int>(height)))
        {
            throw std::runtime_error{"PNG image size mismatch"};
        }

        std::vector<png_bytep> rowPointers(height);

        for (std::size_t y = 0; y < height; ++y)
        {
            auto row = image.getRow(static_cast<int>(y)).data();
            rowPointers[y] = reinterpret_cast<png_bytep>(row);
        }

        png_read_image(m_readPtr, rowPointers.data());
        png_read_end(m_readPtr, nullptr);
    }
    catch(const std::exception&)
    {
        // ignore errors
    }

}

//-------------------------------------------------------------------------

fb32::Image8880
PngDecode::decode()
{
    if (not m_readPtr or not m_infoPtr)
    {
        return fb32::Image8880{};
    }

    try
    {
        const auto width = png_get_image_width(m_readPtr, m_infoPtr);
        const auto height = png_get_image_height(m_readPtr, m_infoPtr);
        fb32::Image8880 image{static_cast<int>(width), static_cast<int>(height)};

        decodeIntoImage(image);

        return image;
    }
    catch(const std::exception&)
    {
        // ignore errors
    }

    return fb32::Image8880{};
}

//-------------------------------------------------------------------------

} // namespace

//=========================================================================

namespace fb32
{

//-------------------------------------------------------------------------

void
decodePng(
    Image8880& image,
    std::span<const uint8_t> data,
    const fb32::RGB8880& background)
{
    PngDecode pd{data, background};
    pd.decodeIntoImage(image);
}

//-------------------------------------------------------------------------

Image8880
readPng(
    const std::string& name,
    const fb32::RGB8880& background)
{
    const auto length{std::filesystem::file_size(std::filesystem::path(name))};

    std::ifstream ifs{name, std::ios_base::binary};
    std::vector<uint8_t> buffer(length);
    ifs.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    PngDecode pd{buffer, background};

    return pd.decode();
}

//-------------------------------------------------------------------------

}constexpr const char* extensions[] = {".jpg", ".jpeg", ".png", ".qoi"};

