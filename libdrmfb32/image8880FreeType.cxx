
//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2023 Andrew Duncan
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

#include "image8880FreeType.h"

#include <stdexcept>

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

Image8880FreeType::Image8880FreeType(
    const std::string& fontFile,
    int pixelSize)
:
    Image8880FreeType()
{
    if (FT_Init_FreeType(&m_library) != 0)
    {
        throw std::invalid_argument("FreeType initialization faied");
    }

    if (FT_New_Face(m_library, fontFile.c_str(), 0, &m_face) != 0)
    {
        throw std::invalid_argument("FreeType could not open " + fontFile);
    }

    setPixelSize(pixelSize);
}

//-------------------------------------------------------------------------

Image8880FreeType::~Image8880FreeType()
{
    FT_Done_FreeType(m_library);
}

//-------------------------------------------------------------------------

std::string
Image8880FreeType::getFontFamilyName() const noexcept
{
    return m_face->family_name;
}

//-------------------------------------------------------------------------

std::string
Image8880FreeType::getFontStyleName() const noexcept
{
    return m_face->style_name;
}

//-------------------------------------------------------------------------

int
Image8880FreeType::getPixelHeight() const noexcept
{
    return (m_face->size->metrics.ascender +
            abs(m_face->size->metrics.descender)) >> 6;
}

//-------------------------------------------------------------------------

int
Image8880FreeType::getPixelWidth() const noexcept
{
    return m_face->size->metrics.max_advance >> 6;
}

//-------------------------------------------------------------------------

std::optional<char>
Image8880FreeType::getCharacterCode(Interface8880Font::CharacterCode code) const noexcept
{
    switch (code)
    {
        using enum Interface8880Font::CharacterCode;

    case DEGREE_SYMBOL:

        return char(0xB0);
    }

    return {};
}

//-------------------------------------------------------------------------

bool
Image8880FreeType::setPixelSize(
    int pixelSize) noexcept
{
    if (pixelSize == m_pixelSize)
    {
        return true;
    }

    if (FT_Set_Pixel_Sizes(m_face, 0, pixelSize) == 0)
    {
        m_pixelSize = pixelSize;

        return true;
    }

    return false;
}

//-------------------------------------------------------------------------

Interface8880Point
Image8880FreeType::drawChar(
    const Interface8880Point& p,
    uint8_t c,
    const RGB8880& rgb,
    Interface8880& image)
{
    return drawString(p, std::string(1, c), rgb, image);
}

//-------------------------------------------------------------------------

Interface8880Point
Image8880FreeType::drawChar(
    const Interface8880Point& p,
    uint8_t c,
    uint32_t rgb,
    Interface8880& image)
{
    return drawChar(p, c, RGB8880(rgb), image);
}

//-------------------------------------------------------------------------

Interface8880Point
Image8880FreeType::drawWideChar(
    const Interface8880Point& p,
    uint32_t c,
    const RGB8880& rgb,
    Interface8880& image)
{
    Interface8880Point position{p};
    position.incrY(m_face->size->metrics.ascender >> 6);
    const auto glyph_index{FT_Get_Char_Index(m_face, c)};

    if (FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER) == 0)
    {
        const auto slot{m_face->glyph};

        drawChar(position.x() + slot->bitmap_left,
                 position.y() - slot->bitmap_top,
                 slot->bitmap,
                 rgb,
                 image);

        position.incrX(slot->advance.x >> 6);
    }

    position.incrY(-(m_face->size->metrics.ascender >> 6));
    return position;
}

//-------------------------------------------------------------------------

Interface8880Point
Image8880FreeType::drawString(
    const Interface8880Point& p,
    std::string_view sv,
    const RGB8880& rgb,
    Interface8880& image)
{
    Interface8880Point position{p};
    position.incrY(m_face->size->metrics.ascender >> 6);

    const auto slot{m_face->glyph};
    const auto use_kerning{FT_HAS_KERNING(m_face)};
    FT_UInt previous{0};

    for (const auto c : sv)
    {
        if (c == '\n')
        {
            position.set(p.x(), position.y() + getPixelHeight());
        }
        else
        {
            const auto glyph_index{FT_Get_Char_Index(m_face, c)};

            if (use_kerning and previous and glyph_index)
            {
                FT_Vector delta;

                FT_Get_Kerning(m_face,
                               previous,
                               glyph_index,
                               ft_kerning_default,
                               &delta);

                position.incrX(delta.x >> 6);
            }

            if (FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER) == 0)
            {
                const auto slot{m_face->glyph};

                drawChar(position.x() + slot->bitmap_left,
                         position.y() - slot->bitmap_top,
                         slot->bitmap,
                         rgb,
                         image);

                position.incrX(slot->advance.x >> 6);
                previous = glyph_index;
            }
        }
    }

    //-----------------------------------------------------------------

    const auto advance = slot->bitmap.width - (slot->advance.x >> 6);

    if (advance > 0)
    {
        position.incrX(advance);
    }

    position.incrY(-(m_face->size->metrics.ascender >> 6));

    return position;
}

//-------------------------------------------------------------------------

Interface8880Point
Image8880FreeType::drawString(
    const Interface8880Point& p,
    std::string_view sv,
    uint32_t rgb,
    Interface8880& image)
{
    return drawString(p, sv, RGB8880(rgb), image);
}

//-------------------------------------------------------------------------

void
Image8880FreeType::drawChar(
        int xOffset,
        int yOffset,
        const FT_Bitmap& bitmap,
        const RGB8880& rgb,
        Interface8880& image)
{
    for (unsigned j = 0 ; j < bitmap.rows ; ++j)
    {
        const auto row{bitmap.buffer + (j * bitmap.pitch)};

        for (unsigned i = 0 ; i < bitmap.width ; ++i)
        {
            if (row[i])
            {
                const Interface8880Point p{static_cast<int>(i + xOffset),
                                       static_cast<int>(j + yOffset)};
                auto background{image.getPixelRGB(p)};

                if (background)
                {
                    image.setPixelRGB(p, rgb.blend(row[i], *background));
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

} // namespace fb32

